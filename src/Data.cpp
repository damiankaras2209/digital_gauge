#include "Data.h"

#include <cmath>

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 0;

DataClass Data;


DataClass::DataClass() {

}

void DataClass::canReset(MCP2515* mcp) {
    MCP2515::ERROR error = mcp->reset();
    if(error == MCP2515::ERROR_OK) {
//        Log.logf("MCP2515 good");
        mcp->setBitrate(CAN_500KBPS, MCP_8MHZ);
        mcp->setListenOnlyMode();
    } else {
        Log.logf("MCP2515 error: %d", error);
    }
}

void DataClass::POST() {
    std::fill(status, status + D_LAST - 1, false);

    Log.logf("POST start");

    status[D_FT5436] = touch.init(&Serial);
    status[D_DS3231] = rtc.begin();
    status[D_ADS1115] = ads.begin();
    status[D_MCP2515] = mcp.reset() == MCP2515::ERROR_OK;
    status[D_MCP23008] = mcp23008.begin_I2C();
    Wire.beginTransmission(0x50);
    status[D_A24C32] = (Wire.endTransmission() == 0);

    for(int i=0; i<D_LAST; i++) {
        Log.logf("%s... %s\n", deviceName[i].c_str(), status[i] ? "good" : "fail");
    }

    Log.logf("POST completed");
}

void DataClass::init() {

    data = DataStruct();
    //    data.dataDisplaySettings = Settings::getInstance()->dataDisplay;
    data.adsPtr = &ads;
    data.rtcPtr = &rtc;
    data.mcp23X08Ptr = &mcp23008;
    data.mcp2515Ptr = &mcp;
    data.rcPtr = &rc;
    data.dataInput = dataInput;


    if (status[D_DS3231]) {
        readTime(&data);
        if(rtc.lostPower()) {
            Log.logf("RTC lost power");
            rtc.adjust(DateTime(2099, 9, 99, 99, 99, 99));
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

    if(status[D_MCP23008]) {
        for(int i=0; i<8; i++) {
            mcp23008.pinMode(i,OUTPUT);
            mcp23008.digitalWrite(i,LOW);
        }
    }

    rc.enableReceive(3);


    touch.enableInterrupt(33, &(data.lock), 1, 0);

    TaskHandle_t adcHandle;
    if(!xTaskCreatePinnedToCore(adcLoop,
        "adcLoop",
        8*1024,
        &data,
        1,
        &adcHandle,
        0))
        Log.logf("Failed to start adcLoop task");

    if(status[D_MCP2515]) {
        TaskHandle_t canHandle;
        if(!xTaskCreatePinnedToCore(canLoop,
            "canLoop",
            4*1024,
            &data,
            1,
            &canHandle,
            0))
                Log.logf("Failed to start canLoop task");
    }

}


void DataClass::setEvent(SendEvent e) {
    _sendEvent = std::move(e);
}

void DataClass::setCountClients(DataClass::CountClients e) {
    _countClients = std::move(e);
}

double MySqr(double a_fVal) {  return a_fVal*a_fVal; }

unsigned long t18 = 0;

_Noreturn void DataClass::adcLoop(void * pvParameters) {
    Log.logf("%s started on core %d\n", pcTaskGetTaskName(NULL), xPortGetCoreID());

    auto *params = (DataStruct*)pvParameters;


    mu::Parser p;
    double voltage;
    double res;

    try {
        p.DefineVar("v", &voltage);
        p.DefineVar("r", &res);
    } catch (mu::Parser::exception_type &e) {
        Log.logf("Exception: %s\n", e.GetMsg().c_str());
    }

    uint32_t readings[SettingsClass::VOLTAGE + 1][SAMPLES_ADC];
    int readingsTaken[SettingsClass::VOLTAGE + 1];
    for(int i=0; i<SettingsClass::VOLTAGE + 1; i++)
        readingsTaken[i] = 0;

    for(;;) {

//        unsigned long t = millis();



        for(int i=0; i <= SettingsClass::VOLTAGE && Data.status[D_ADS1115]; i++) {
            if(params->dataInput[i].visible) {

                for(int j= SAMPLES_ADC - 1; j > 0; j--)
                    readings[i][j] = readings[i][j - 1];

                if(i<4) {
                    params->lock.lock();
                    readings[i][0] = params->adsPtr->readADC(i);
                    params->lock.release();
                    delay(1);
                } else if(i==4)
                    readings[i][0] = analogReadMilliVolts(36);
                else if(i==5)
                    readings[i][0] = analogReadMilliVolts(39);
                else if(i == SettingsClass::VOLTAGE)
                    readings[i][0] = analogReadMilliVolts(34);

                readingsTaken[i] = min(++readingsTaken[i], SAMPLES_ADC);

//                Serial.print("Raw voltage: ");
//                Serial.print((double)readings[i][0] / 1000.0);
//                Serial.print(" ");
//                Serial.print((double)readings[i][0] * 5.7 / 1000.0);
//                Serial.print("\n");


                uint32_t sum = 0;
//                Log.logf("Readings taken: %d\n", readingsTaken[i]);
                for(int j=0; j < readingsTaken[i]; j++) {
//                    Log.logf(readings[i][j]);
//                    Log.logf(", ");
                    sum += readings[i][j];
                }
//                Log.logf("");

                double avg = (double)sum / readingsTaken[i];


                if(i<4)
                    voltage = params->adsPtr->toVoltage((int16_t)lround(avg));
                else
                    voltage = avg/1000.0;

                params->dataInput[i].voltage = voltage;

//                if(i==Settings::ADS1115_1) {
//                    Log.logf("%s - avg: %f", settings->dataSourceString[i].c_str(), avg);
//                    Log.logf(", voltage: %f", voltage);
//                }

                SettingsClass::Field** input = &(Settings.general[INPUT_BEGIN_BEGIN + i * INPUT_SETTINGS_SIZE]);
                res = input[INPUT_PULLUP_OFFSET]->get<float>() * voltage / (3.3 - voltage) + input[INPUT_SERIES_OFFSET]->get<float>();
                params->dataInput[i].resistance = res;

                auto up = input[INPUT_PULLUP_OFFSET]->get<double>();
                auto down = input[INPUT_PULLDOWN_OFFSET]->get<double>();
                auto series = input[INPUT_SERIES_OFFSET]->get<double>();

                p.DefineVar("up", &up);
                p.DefineVar("down", &down);
                p.DefineVar("series", &series);

//                    Log.logf("%s - voltage: %f", Settings.dataSourceString[i].c_str(), voltage);
//                    Log.logf(", R: %f", res);


                try {
                    p.SetExpr(Settings.general[INPUT_BEGIN_BEGIN + i * INPUT_SETTINGS_SIZE + INPUT_EXPRESSION_OFFSET]->getString());
                    params->dataInput[i].value = (float)p.Eval();
                } catch (mu::Parser::exception_type &e) {
                    Log.logf("Exception at %d: %s, \"%s\"\n", i, e.GetMsg().c_str(), e.GetExpr().c_str());
                }

//                    Log.logf(", value: %f", Settings.general[DATA_BEGIN_BEGIN + i * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->get<float>());
//

//                    params->dataInput[i].value = voltage * 5.7;

//                Log.logf(", inputValue: %f\n", params->inputValue[i]);
            }
        }

        if(Data._countClients != nullptr && Data._sendEvent != nullptr) {
            if(millis() - t18 > 0 && Data._countClients() > 0) {
                std::stringstream ss;
                ss << "{";
                int x=0;
                for(int i=0; i<=SettingsClass::VOLTAGE + 1; i++) {
                    if(params->dataInput[i].visible) {
                        if (x > 0)
                            ss << ",";
                        ss << "\"" << i << "\":" << params->dataInput[i].toString();
                        x++;
                    }
                }
                ss << "}";
                Data._sendEvent("data", ss.str());
                t18 = millis();
            }
        }


        if(Data.status[D_DS3231] && millis()-params->lastRTC > 1000) {
            readTime(params);
        }


        if(Data.status[D_MCP23008]) {
            if (params->canActive) {
                if (Data.data.dataInput[SettingsClass::CAN_RPM].value > 1000 && params->relayState[HEADLIGHTS_PIN] == LOW) {
                    params->lock.lock();
                    Data.mcp23008.digitalWrite(HEADLIGHTS_PIN, HIGH);
                    params->lock.release();
                    params->relayState[HEADLIGHTS_PIN] = HIGH;
                }
            } else if(false) { //skip turning headlights off
                //to do
                //persistent logging of time without frames
                if (params->relayState[HEADLIGHTS_PIN] == HIGH) {
                    params->lock.lock();
                    Data.mcp23008.digitalWrite(HEADLIGHTS_PIN, LOW);
                    params->lock.release();
                    params->relayState[HEADLIGHTS_PIN] = LOW;
                }
            }

            if (!params->relayState[THROTTLE_POWER_PIN] && params->shouldToggleValve && millis() - params->lastValveChange > THROTTLE_VALVE_DELAY) {
                params->lock.lock();
                Data.mcp23008.digitalWrite(THROTTLE_PIN, !Settings.state.throttleState);
                Data.mcp23008.digitalWrite(THROTTLE_POWER_PIN, HIGH);
                bool newState = Data.mcp23008.digitalRead(THROTTLE_PIN);
                params->lock.release();

                params->shouldToggleValve = false;
                params->lastValvePowerOn = millis();
                params->relayState[THROTTLE_PIN] = newState;
                params->relayState[THROTTLE_POWER_PIN] = HIGH;

                Settings.state.throttleState = newState;
                Settings.saveState();
            }

            if (params->relayState[THROTTLE_POWER_PIN] && millis() - params->lastValvePowerOn > THROTTLE_VALVE_POWER_ON_TIME) {
                params->lock.lock();
                Data.mcp23008.digitalWrite(THROTTLE_POWER_PIN, LOW);
                Data.mcp23008.digitalWrite(THROTTLE_PIN, LOW);
                params->lock.release();
                params->relayState[THROTTLE_PIN] = LOW;
                params->relayState[THROTTLE_POWER_PIN] = LOW;
                params->lastValveChange = millis();

            }


        }

        delay(1);
    }
}

_Noreturn void DataClass::canLoop(void * pvParameters) {
    Log.logf("%s started on core %\n", pcTaskGetTaskName(NULL), xPortGetCoreID());
//    Log.logf(" started on core ");
//    Log.logf(xPortGetCoreID());

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

        if(millis() - params->lastFrame > CAN_INACTIVITY_TRESHOLD) {
            params->canActive = false;
            Log.logf("Can lost");
            if(millis() - params->lastCanInit > CAN_REINIT_AFTER) {
                params->lastCanInit = millis();
                canReset(params->mcp2515Ptr);
            }
            delay(500);
        }

        MCP2515::ERROR err = params->mcp2515Ptr->readMessage(&canMsg);

        if (err == MCP2515::ERROR_OK) {

            if(!params->canActive) {
                params->canActive = true;
                Log.logf("Can up after %d ms of inactivity", millis() - params->lastFrame);
            }

            params->lastFrame = millis();

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
//            Log.logf(ss.str().c_str());


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

                    params->dataInput[SettingsClass::CAN_STEERING_ANGLE].value = (float) sign * val / 10;
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

                    params->dataInput[SettingsClass::CAN_RPM].value = (float) sumRPM / SAMPLES_CAN / 4;
                    params->dataInput[SettingsClass::CAN_SPEED].value = (float)  sumSpeed / SAMPLES_CAN * 2;
                    params->dataInput[SettingsClass::CAN_GAS].value = (float) sumGas / SAMPLES_CAN / 51200 * 100;
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
                        params->dataInput[SettingsClass::CAN_HB].value = (float)((canMsg.data[6] & 0x20) >> 5);
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

DataClass::DataInput *DataClass::getDataInput() {
    return dataInput;
}

DateTime DataClass::getTime() {
    return data.now;
}



void DataClass::readTime(DataClass::DataStruct *params) {
    params->lock.lock();
    auto now = params->rtcPtr->now();
    params->lock.release();
//            std::stringstream s;
//            s << std::setfill('0') << std::setw(2) << ((String)params->now.hour()).c_str() << ":" << std::setw(2) << ((String)params->now.minute()).c_str()  << ":" << std::setw(2) << ((String)params->now.second()).c_str();
//            Log.logf("RTC: %s\n", s.str().c_str());
    params->lastRTC = millis();

    int year = now.year();
    DateTime begin(year, 3, Date::findLastSunday(year, 3), 2, 0, 0);
    DateTime end(year, 10, Date::findLastSunday(year, 10), 2, 0, 0);
    if(now >= begin && now < end)
        now = now + TimeSpan(3600);

    if(now.hour() > 24 || now.minute() > 60 || now.day() > 31 || now.month() > 13 || now.year() > 2098)
        now = DateTime(2099, 9, 99, 99, 99);

    params->now = now;
}

int DataClass::adjustTime(DataStruct *params) {

    Log.logf("Getting time from server");
    struct tm timeinfo;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if(!getLocalTime(&timeinfo)){
        Log.logf("Failed to obtain time from server");
        return D_FAIL;
    }
    params->lock.lock();

    timeinfo.tm_mon += 1;
    timeinfo.tm_year += 1900;
    params->rtcPtr->adjust(DateTime(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
     Log.logf("Got: %d.%d.%d %d:%d:%d\n",
             timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    params->lock.release();
    return D_SUCCESS;
}
