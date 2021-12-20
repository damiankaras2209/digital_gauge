#include "Data.h"

#include <cmath>

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


Data::Data() {

}


void Data::init() {

    data = DataStruct();
//    data.dataDisplaySettings = Settings::getInstance()->dataDisplay;
    data.adsPtr = &ads;
    data.rtcPtr = &rtc;
    data.mcp23X08Ptr = &mcp23008;
    data.mcp2515Ptr = &mcp;
    data.rcPtr = &rc;

    if (!rtc.begin()) {
        Log.log("Couldn't find RTC");
        data.RTCAvailable = false;
    } else {
        Log.log("RTC good");
        data.RTCAvailable = true;
        data.now = rtc.now();
        data.lastRTC = millis();
        if(rtc.lostPower()) {
            Log.log("RTC lost power");
            rtc.adjust(DateTime(22, 2, 22, 22, 22, 22));
        }
    }

    mcp.reset();
    mcp.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp.setListenOnlyMode();


    if(!ads.begin())
        Log.log("ADS1115 not found");
    else {
        Log.log("ADS1115 found");
        ads.setGain(1);
        ads.setDataRate(7);
    }



    rc.enableReceive(3);

    if(data.RTCAvailable) {
        TaskHandle_t rtcAdjustHandle;
        if(!xTaskCreatePinnedToCore(adjustRTCTask,
             "adjustRTC",
             4*1024,
             &data,
             3,
             &rtcAdjustHandle,
             0))
                 Log.log("Failed to start rtc adjust task");
    }

    TaskHandle_t adcHandle;
    if(!xTaskCreatePinnedToCore(adcLoop,
        "adcLoop",
        4*1024,
        &data,
        1,
        &adcHandle,
        0))
        Log.log("Failed to start adcLoop task");

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

_Noreturn void Data::adcLoop(void * pvParameters) {
    Log.logf("%s started on core %d\n", pcTaskGetTaskName(NULL), xPortGetCoreID());
//    Log.log(" started on core ");
//    Log.log(xPortGetCoreID());

    DataStruct *params = (DataStruct*)pvParameters;
    Settings *settings = Settings::getInstance();

    for(;;) {
        while(params->i2cBusy)
           delay(1);
        params->i2cBusy = true;

        uint32_t readings[Settings::VOLTAGE+1][SAMPLES_ADC];

        for(int i=0; i<=Settings::VOLTAGE && params->adsPtr->isConnected(); i++) {
            if(settings->dataDisplay[i].enable) {

                for(int j= SAMPLES_ADC - 1; j > 0; j--)
                    readings[i][j] = readings[i][j - 1];

                if(i<4)
                    readings[i][0] = params->adsPtr->readADC(i);
                else if(i==4)
                    readings[i][0] = analogReadMilliVolts(36);
                else if(i==5)
                    readings[i][0] = analogReadMilliVolts(39);
                else if(i==Settings::VOLTAGE)
                    readings[i][0] = analogReadMilliVolts(34);

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

                if(i<Settings::VOLTAGE) {

                    volatile Settings::InputSettings *input = &(settings->input[i]);
                    float res = input->r * voltage / (3.3 - voltage);

//                    Log.logf("%s - voltage: %f", settings->dataSourceString[i].c_str(), voltage);
//                    Log.logf(", R: %f", res);

//                    Log.logf(", R: %f\n", res);

                    switch(input->type){
                        case Settings::Logarithmic: settings->dataDisplay[i].value = input->beta * (25.0 + 273.15) / (input->beta + ((25.0 + 273.15) * log(res / input->r25))) - 273.15; break;
                        case Settings::Linear: settings->dataDisplay[i].value = (res - input->rmin) / (input->rmax - input->rmin) * input->maxVal; break;
                        case Settings::Voltage: settings->dataDisplay[i].value = voltage; break;
                    }

//                    if(i==Settings::ADS1115_1) {
//                        Log.logf(", value: %f\n", settings->dataDisplay[i].value);
//                    }

                } else if(i == Settings::VOLTAGE)
                    settings->dataDisplay[i].value = voltage * 5.7;

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
        delay(10);
    }
}

_Noreturn void Data::canLoop(void * pvParameters) {
    Log.logf("%s started on core %\n", pcTaskGetTaskName(NULL), xPortGetCoreID());
//    Log.log(" started on core ");
//    Log.log(xPortGetCoreID());

    struct can_frame canMsg{};
    DataStruct *params = (DataStruct*)pvParameters;
    Settings *settings = Settings::getInstance();

    int rpm[3];

    for(;;) {

        MCP2515::ERROR err = params->mcp2515Ptr->readMessage(&canMsg);

        if (err == MCP2515::ERROR_OK) {
            switch (canMsg.can_id ) {
                case CAN_ID_RPM:
                    rpm[2] = rpm[1];
                    rpm[1] = rpm[0];
                    rpm[0] = (canMsg.data[0]*256 + canMsg.data[1])/4;
                    settings->dataDisplay[Settings::CAN_RPM].value = (rpm[0]+rpm[1]+rpm[2])/3;
                    break;
            }
        } else {
//                if( err != MCP2515::ERROR_NOMSG) {
//                     Log.log("Error: ");
//                     Log.log(err);
//                     Log.log("\n");
//                }
        }
        delay(10);
    }
}


DateTime Data::getTime() {
    return data.now;
}

void Data::adjustRTCTask(void * pvParameters) {
    Log.logf("%s started on core %d\n", pcTaskGetTaskName(NULL), xPortGetCoreID());
//    Log.log(" started on core ");
//    Log.log(xPortGetCoreID());

    while(WiFi.status() != WL_CONNECTED) {
        delay(100);
    }

    auto params = (DataStruct*)pvParameters;
    while(params->i2cBusy)
        delay(1);
    params->i2cBusy = true;

    Log.log("Getting time from server");
    struct tm timeinfo;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if(!getLocalTime(&timeinfo)){
        Log.log("Failed to obtain time from server");
    } else {
        Log.log("Success ");
//        Log.log(&timeinfo, "%A, %B %d %Y %H:%M:%S");
//        Log.logf("%d.%d.%d %d:%d:%d\n", timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
        ((DataStruct*)pvParameters)->rtcPtr->adjust(DateTime(timeinfo.tm_year, timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
//        DateTime now  = ((DataStruct*)pvParameters)->rtcPtr->now();
//        Log.logf("Adjusted to: %d:%d\n", now.hour(), now.minute());
    }

    params->i2cBusy = false;
    vTaskDelete(NULL);
}


