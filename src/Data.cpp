#include "Data.h"

#include <cmath>

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

DataClass Data;


DataClass::DataClass() {

}

void DataClass::canReset(MCP2515* mcp) {
    MCP2515::ERROR error = mcp->reset();
    if(error == MCP2515::ERROR_OK) {
//        Log.log("MCP2515 good");
        mcp->setBitrate(CAN_500KBPS, MCP_8MHZ);
        mcp->setListenOnlyMode();
    } else {
        Log.logf("MCP2515 error: %d", error);
    }
}

void DataClass::POST() {
    std::fill(status, status + D_LAST - 1, false);

    Log.log("POST start");

    status[D_FT5436] = touch.init(&Serial);
    status[D_DS3231] = rtc.begin();
    status[D_ADS1115] = ads.begin();
    status[D_MCP2515] = mcp.reset() == MCP2515::ERROR_OK;
    status[D_MCP23008] = mcp23008.begin_I2C();

    for(int i=0; i<D_LAST; i++) {
        Log.logf("%s... %s\n", deviceName[i].c_str(), status[i] ? "good" : "fail");
    }

    Log.log("POST completed");
}

void DataClass::init() {

    data = DataStruct();
    //    data.dataDisplaySettings = Settings::getInstance()->dataDisplay;
    data.adsPtr = &ads;
    data.rtcPtr = &rtc;
    data.mcp23X08Ptr = &mcp23008;
    data.mcp2515Ptr = &mcp;
    data.rcPtr = &rc;

    if (!status[D_DS3231]) {
        data.RTCAvailable = false;
    } else {
        data.RTCAvailable = true;
        data.now = rtc.now();
        data.lastRTC = millis();
        if(rtc.lostPower()) {
            Log.log("RTC lost power");
            rtc.adjust(DateTime(2222, 2, 22, 22, 22, 22));
        }
    }

    if(status[D_MCP2515]) {
        mcp.setBitrate(CAN_500KBPS, MCP_8MHZ);
        mcp.setListenOnlyMode();
    }

    if(status[D_ADS1115]) {
        ads.setGain(1);
        ads.setDataRate(7);
    }

    rc.enableReceive(3);


    touch.enableInterrupt(33, &(data.i2cBusy), 1, 0);

    TaskHandle_t adcHandle;
    if(!xTaskCreatePinnedToCore(adcLoop,
        "adcLoop",
        8*1024,
        &data,
        1,
        &adcHandle,
        0))
        Log.log("Failed to start adcLoop task");

    if(status[D_MCP2515]) {
        TaskHandle_t canHandle;
        if(!xTaskCreatePinnedToCore(canLoop,
            "canLoop",
            4*1024,
            &data,
            1,
            &canHandle,
            0))
                Log.log("Failed to start canLoop task");
    }

}

_Noreturn void DataClass::adcLoop(void * pvParameters) {
    Log.logf("%s started on core %d\n", pcTaskGetTaskName(NULL), xPortGetCoreID());
//    Log.log(" started on core ");
//    Log.log(xPortGetCoreID());

    DataStruct *params = (DataStruct*)pvParameters;

    for(;;) {
        while(params->i2cBusy)
           delay(1);
        params->i2cBusy = true;

        uint32_t readings[SettingsClass::VOLTAGE + 1][SAMPLES_ADC];

        for(int i=0; i <= SettingsClass::VOLTAGE && params->adsPtr->isConnected(); i++) {
            if(Settings.general[DATA_BEGIN_BEGIN + i * DATA_SETTINGS_SIZE + DATA_ENABLE_OFFSET]->get<bool>()) {

                for(int j= SAMPLES_ADC - 1; j > 0; j--)
                    readings[i][j] = readings[i][j - 1];

                if(i<4)
                    readings[i][0] = params->adsPtr->readADC(i);
                else if(i==4)
                    readings[i][0] = analogReadMilliVolts(36);
                else if(i==5)
                    readings[i][0] = analogReadMilliVolts(39);
                else if(i == SettingsClass::VOLTAGE)
                    readings[i][0] = analogReadMilliVolts(34);

//                Serial.print("Raw voltage: ");
//                Serial.print((double)readings[i][0] / 1000.0);
//                Serial.print(" ");
//                Serial.print((double)readings[i][0] * 5.7 / 1000.0);
//                Serial.print("\n");


                uint32_t sum = 0;
//                Log.log(i);
//                Log.log(" ");
                for(int j=0; j < SAMPLES_ADC; j++) {
//                    Log.log(readings[i][j]);
//                    Log.log(", ");
                    sum += readings[i][j];
                }
//                Log.log("");

                double avg = (double)sum / SAMPLES_ADC;

                double voltage;
                if(i<4)
                    voltage = params->adsPtr->toVoltage((int16_t)lround(avg));
                else
                    voltage = avg/1000.0;

//                if(i==Settings::ADS1115_1) {
//                    Log.logf("%s - avg: %f", settings->dataSourceString[i].c_str(), avg);
//                    Log.logf(", voltage: %f", voltage);
//                }

                if(i < SettingsClass::VOLTAGE) {


                    SettingsClass::Field** input = &(Settings.general[INPUT_BEGIN_BEGIN + i * INPUT_SETTINGS_SIZE]);
                    float res = input[INPUT_R_OFFSET]->get<float>() * voltage / (3.3 - voltage);

//                    Log.logf("%s - voltage: %f", settings->dataSourceString[i].c_str(), voltage);
//                    Log.logf(", R: %f", res);

//                    Log.logf(", R: %f\n", res);

                    switch(input[INPUT_TYPE_OFFSET]->get<int>()){
                        case Logarithmic: Settings.general[DATA_BEGIN_BEGIN + i * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->set(input[INPUT_BETA_OFFSET]->get<float>() * (25.0 + 273.15) / (input[INPUT_BETA_OFFSET]->get<float>() + ((25.0 + 273.15) * log(res / input[INPUT_R25_OFFSET]->get<float>()))) - 273.15); break;
                        case Linear:      Settings.general[DATA_BEGIN_BEGIN + i * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->set((res - input[INPUT_RMIN_OFFSET]->get<float>()) / (input[INPUT_RMAX_OFFSET]->get<float>() - input[INPUT_RMIN_OFFSET]->get<float>()) * input[INPUT_MAXVAL_OFFSET]->get<float>()); break;
                        case Voltage:     Settings.general[DATA_BEGIN_BEGIN + i * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->set(voltage); break;
                    }

//                    if(i==Settings::ADS1115_1) {
//                        Log.logf(", value: %f\n", settings->dataDisplay[i].value);
//                    }

                } else if(i == SettingsClass::VOLTAGE)
                    Settings.general[DATA_BEGIN_BEGIN + i * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->set(voltage * 5.7);

//                Log.logf(", inputValue: %f\n", params->inputValue[i]);
            }
        }

        if(params->RTCAvailable && millis()-params->lastRTC > 1000) {
            params->now = params->rtcPtr->now();
//            std::stringstream s;
//            s << std::setfill('0') << std::setw(2) << ((String)params->now.hour()).c_str() << ":" << std::setw(2) << ((String)params->now.minute()).c_str()  << ":" << std::setw(2) << ((String)params->now.second()).c_str();
//            Log.logf("RTC: %s\n", s.str().c_str());
            params->lastRTC = millis();
        }

        if (params->rcPtr->available()) {

            Log.log("Received ");
            Log.log(params->rcPtr->getReceivedValue() );
            Log.log(" / ");
            Log.log(params->rcPtr->getReceivedBitlength() );
            Log.log("bit ");
            Log.log("Protocol: ");
            Log.log( params->rcPtr->getReceivedProtocol() );

            params->rcPtr->resetAvailable();
        }

        params->i2cBusy = false;
        delay(1);
    }
}


ulong lastFrame = 0;
ulong lastCanInit = 0;
_Noreturn void DataClass::canLoop(void * pvParameters) {
    Log.logf("%s started on core %\n", pcTaskGetTaskName(NULL), xPortGetCoreID());
//    Log.log(" started on core ");
//    Log.log(xPortGetCoreID());

    struct can_frame canMsg{};
    DataStruct *params = (DataStruct*)pvParameters;

    uint32_t samplesRPM[SAMPLES_CAN];
    uint32_t samplesGas[SAMPLES_CAN];
    uint32_t samplesSpeed[SAMPLES_CAN];

    struct can_frame msg{};
    msg.can_id = 0x2137;
    msg.can_dlc = 2;
    msg.data[0] = 0x21;
    msg.data[1] = 0x37;

    for(;;) {

//        params->mcp2515Ptr->sendMessage(&msg);

        if(millis() - lastFrame > 1000) {
//            Log.log("Can lost");
            if(millis() - lastCanInit > 1000) {
//                Log.log("Try to init");
                lastCanInit = millis();
                canReset(params->mcp2515Ptr);
            }
            delay(500);
        }

        MCP2515::ERROR err = params->mcp2515Ptr->readMessage(&canMsg);

        if (err == MCP2515::ERROR_OK) {

            lastFrame = millis();

//            std::stringstream ss;
//
//
//            bool ext = canMsg.can_id & CAN_EFF_FLAG;
//            bool rtr = canMsg.can_id & CAN_RTR_FLAG;
//
//            ss << "EXT: " << ext << " RTR: " << rtr << " ID HEX: ";
//            ss << "0x" << std::uppercase << std::hex << (canMsg.can_id & (ext ? CAN_EFF_MASK : CAN_SFF_MASK));
//            ss << " Data: ";
//
//            for(int i=0; i<canMsg.can_dlc; i++) {
//                ss << "0x" << std::uppercase << std::hex << canMsg.data[i] << " ";
////                Serial.println(canMsg.data[i], HEX);
//            }
//
//            Log.log(ss.str().c_str());


            switch (canMsg.can_id ) {
                case CAN_ID_STEERING_ANGLE: {

//                    Serial.print("SW data: ");
//                    for(int i=0; i<canMsg.can_dlc; i++) {
//                        Serial.print(canMsg.data[i], HEX); // print DLC
//                        Serial.print(" ");
//                    }
//                    Serial.print("\n");

                    int raw = canMsg.data[1] << 8 | canMsg.data[0];
                    int sign = raw > 0x8888 ? 1 : -1;
                    int val = sign==1 ? 0xffff - raw : raw;

                    Settings.general[DATA_BEGIN_BEGIN + SettingsClass::DataSource::CAN_STEERING_ANGLE * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->set((float) sign * val / 10); //checked
//                    Serial.println(settings->dataDisplay[Settings::CAN_STEERING_ANGLE].value);
                    break;
                }
                case CAN_ID_RPM_SPEED_GAS: {

//                    Serial.print("RPM_SPEED_GAS data: ");
//                    for(int i=0; i<canMsg.can_dlc; i++) {
//                        Serial.print(canMsg.data[i], HEX); // print DLC
//                        Serial.print(" ");
//                    }
//                    Serial.print("\n");

                    for (int j = SAMPLES_CAN - 1; j > 0; j--) {
                        samplesRPM[j] = samplesRPM[j - 1];
                        samplesGas[j] = samplesGas[j - 1];
                        samplesSpeed[j] = samplesSpeed[j - 1];
                    }

                    samplesRPM[0] = canMsg.data[0] * 256 + canMsg.data[1];
                    samplesGas[0] = canMsg.data[6] * 256 + canMsg.data[7];
                    samplesSpeed[0] = canMsg.data[4];

                    uint32_t sumRPM = 0;
                    uint32_t sumGas = 0;
                    uint32_t sumSpeed = 0;
                    for (int j = 0; j < SAMPLES_CAN; j++){
                        sumRPM += samplesRPM[j];
                        sumGas += samplesGas[j];
                        sumSpeed += samplesSpeed[j];
                    }

                    Settings.general[DATA_BEGIN_BEGIN + SettingsClass::DataSource::CAN_RPM * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->set((float) sumRPM / SAMPLES_CAN / 4); //checked
                    Settings.general[DATA_BEGIN_BEGIN + SettingsClass::DataSource::CAN_SPEED * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->set((float)  sumSpeed / SAMPLES_CAN * 2); //checked
                    Settings.general[DATA_BEGIN_BEGIN + SettingsClass::DataSource::CAN_GAS * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->set((float) sumGas / SAMPLES_CAN / 51200 * 100); //checked
//                    Serial.printf("rpm: %f", settings->dataDisplay[Settings::CAN_RPM].value);
//                    Serial.printf("speed: %f", settings->dataDisplay[Settings::CAN_SPEED].value);
//                    Serial.printf("gas: %f", settings->dataDisplay[Settings::CAN_GAS].value);

                    break;
                }
                case CAN_ID_HB: {
//                    Serial.print("HB data: ");
//                    for(int i=0; i<canMsg.can_dlc; i++) {
//                        Serial.print(canMsg.data[i], HEX); // print DLC
//                        Serial.print(" ");
//                    }
//                    Serial.print("\n");

                    int raw = canMsg.data[0] << 16 | canMsg.data[1] << 8 | canMsg.data[2];

                    if(raw != 0x0) {
                        Settings.general[DATA_BEGIN_BEGIN + SettingsClass::DataSource::CAN_HB * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->set((canMsg.data[6] & 0x20) >> 5);
//                        Serial.printf(" HB: %f\n", settings->dataDisplay[Settings::CAN_HB].value);
                    }

                }
            }
//                    Serial.printf("RPM: %f, time: %ld\n", settings->dataDisplay[Settings::CAN_RPM].value, millis()-canLoopTime);
//                    canLoopTime = millis();

        } else {
            if(err != MCP2515::ERROR_NOMSG) {
                 Log.logf("Error: %d", err);
            }
        }
        delay(1); //1
    }
}


DateTime DataClass::getTime() {
    return data.now;
}

int DataClass::adjustTime(DataStruct *params) {

    Log.log("Getting time from server");
    struct tm timeinfo;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if(!getLocalTime(&timeinfo)){
        Log.log("Failed to obtain time from server");
        return D_FAIL;
    }


    Log.log("ree");

    while(params->i2cBusy)
        delay(1);
    params->i2cBusy = true;


    Log.log("reee2");

    timeinfo.tm_mon += 1;
    timeinfo.tm_year += 1900;
    params->rtcPtr->adjust(DateTime(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
    DateTime now  = params->rtcPtr->now();
    params->lastRTC = millis();
    Log.logf("Got: %d.%d.%d %d:%d:%d, adjusted to: %d.%d.%d %d:%d:%d\n",
             timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec,
             now.day(), now.month(), now.year(), now.hour(), now.minute(), now.second());

    params->i2cBusy = false;
    return D_SUCCESS;
}