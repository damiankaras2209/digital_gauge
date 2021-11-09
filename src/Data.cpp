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
    data.touchPtr = &touch;
    data.mcp23X08Ptr = &mcp23008;
    data.mcp2515Ptr = &mcp;
    data.rcPtr = &rc;

    if (!rtc.begin())
        Serial.println("Couldn't find RTC");
    else {
        Serial.println("RTC good");
        if(rtc.lostPower()) {
            Serial.println("RTC lost power");
            rtc.adjust(DateTime(1, 1, 1, 22, 22, 22));
        }
    }


    TaskHandle_t rtcAdjustHandle;
    Serial.print(xTaskCreatePinnedToCore(adjustRTCTask,
                                    "adjustRTC",
                                    4*1024,
                                    &data,
                                    3,
                                    &rtcAdjustHandle,
                                    0) ? "" : "Failed to start rtc adjust task\n");



    mcp.reset();
    mcp.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp.setListenOnlyMode();


    if(!ads.begin())
        Serial.println("ADS1115 not found");
    else {
        Serial.println("ADS1115 found");
        ads.setGain(1);
        ads.setDataRate(7);
    }



    data.GxFT5436Available = touch.init(&Serial);

    TaskHandle_t adcHandle;
    Serial.print(xTaskCreatePinnedToCore(adcLoop,
                                    "adcLoop",
                                    4*1024,
                                    &data,
                                    1,
                                    &adcHandle,
                                    0) ? "" : "Failed to start adcLoop task\n");


    TaskHandle_t canHandle;
    Serial.print( xTaskCreatePinnedToCore(canLoop,
                                            "can",
                                            32*1024,
                                            &data,
                                            1,
                                            &canHandle, 0) ? "" : "Failed to start can task\n");

    rc.enableReceive(3);

}



_Noreturn void Data::adcLoop(void * pvParameters) {
    Serial.print(pcTaskGetTaskName(NULL));
    Serial.print(" started on core ");
    Serial.println(xPortGetCoreID());

    DataStruct *params = (DataStruct*)pvParameters;
    Settings *settings = Settings::getInstance();

    for(;;) {
        while(params->i2cBusy)
           delay(1);
        params->i2cBusy = true;

        uint32_t readings[Settings::VOLTAGE+1][SAMPLES];

        for(int i=0; i<=Settings::VOLTAGE && params->adsPtr->isConnected(); i++) {
            if(settings->dataDisplay[i].enable) {

                for(int j=SAMPLES-1; j>0; j--)
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
//                Serial.print(i);
//                Serial.print(" ");
                for(int j=0; j<SAMPLES; j++) {
//                    Serial.print(readings[i][j]);
//                    Serial.print(", ");
                    sum += readings[i][j];
                }
//                Serial.println("");

                double avg = (double)sum/SAMPLES;

                double voltage;
                if(i<4)
                    voltage = params->adsPtr->toVoltage((int16_t)lround(avg));
                else
                    voltage = avg/1000.0;

//                if(i!=Settings::VOLTAGE) {
//                    Serial.printf("%s - avg: %f", settings->dataSourceString[i].c_str(), avg);
//                    Serial.printf(", voltage: %f", voltage);
//                }

                if(i<Settings::VOLTAGE) {

                    volatile Settings::InputSettings *input = &(settings->input[i]);
                    float res = input->r * voltage / (3.29 - voltage);

//                    Serial.printf(", R: %f\n", res);

                    switch(input->type){
                        case Settings::Logarithmic: settings->dataDisplay[i].value = input->beta * (25.0 + 273.15) / (input->beta + ((25.0 + 273.15) * log(res / input->r25))) - 273.15; break;
                        case Settings::Linear: settings->dataDisplay[i].value = (res - input->rmin) / (input->rmax - input->rmin) * input->maxVal; break;
                        case Settings::Voltage: settings->dataDisplay[i].value = voltage; break;
                    }

                } else if(i == Settings::VOLTAGE)
                    settings->dataDisplay[i].value = voltage * 5.7;

//                Serial.printf(", inputValue: %f\n", params->inputValue[i]);
            }
        }

        Wire.beginTransmission(DS3231_ADDRESS);
        if(Wire.endTransmission() == 0)
            params->now = params->rtcPtr->now();

        if (params->rcPtr->available()) {

            Serial.print("Received ");
            Serial.print(params->rcPtr->getReceivedValue() );
            Serial.print(" / ");
            Serial.print(params->rcPtr->getReceivedBitlength() );
            Serial.print("bit ");
            Serial.print("Protocol: ");
            Serial.println( params->rcPtr->getReceivedProtocol() );

            params->rcPtr->resetAvailable();
        }

        params->i2cBusy = false;
        delay(10);
    }
}

_Noreturn void Data::canLoop(void * pvParameters) {


    struct can_frame canMsg;
    DataStruct *params = (DataStruct*)pvParameters;

    int rpm[3];

    for(;;) {
//        struct can_frame frame;
//        frame.can_id = 0x213;
//        frame.can_dlc = 4;
//        frame.data[0] = 0xFF;
//        frame.data[1] = 0xFF;
//        frame.data[2] = 0xFF;
//        frame.data[3] = 0xF0;
    //  mcp2515.sendMessage(&frame);

    MCP2515::ERROR err = ((DataStruct*)pvParameters)->mcp2515Ptr->readMessage(&canMsg);

        if (err == MCP2515::ERROR_OK && canMsg.can_id == 0x201) {
//            Serial.print(canMsg.can_id, HEX); // print ID
//            Serial.print(" ");
//            Serial.print(canMsg.can_dlc, HEX); // print DLC
//            Serial.print(" ");

//            for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
//                Serial.print(canMsg.data[i],HEX);
//                Serial.print(" ");
//            }


            rpm[2] = rpm[1];
            rpm[1] = rpm[0];
            rpm[0] = (canMsg.data[0]*256 + canMsg.data[1])/4;


//            Serial.print("RPM: ");
//            Serial.print((rpm[0]+rpm[1]+rpm[2])/3);
//
//            Serial.println();
            ((DataStruct*)pvParameters)->rpm = (rpm[0]+rpm[1]+rpm[2])/3;
        } else {
            // return rpmVal;
            //    if( err != MCP2515::ERROR_NOMSG) {
            //     Serial.print("Error: ");
            //     Serial.print(err);
            //     Serial.print("\n");
            //    }
        }
        delay(10);
    }
}


DateTime Data::getTime() {
    return data.now;
}

void Data::adjustRTCTask(void * pvParameters) {
    Serial.print(pcTaskGetTaskName(NULL));
    Serial.print(" started on core ");
    Serial.println(xPortGetCoreID());
    while(WiFi.status() != WL_CONNECTED) {
        delay(100);
    }
    Serial.println("Getting time from server");
    struct tm timeinfo;
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time from server");
    } else {
        Serial.print("Success ");
        Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
        ((DataStruct*)pvParameters)->rtcPtr->adjust(DateTime(timeinfo.tm_year, timeinfo.tm_mon+1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
//        Serial.println("halo: ");
        DateTime now  = ((DataStruct*)pvParameters)->rtcPtr->now();
//        Serial.println("min: ");
//        Serial.println(now.minute());
//        Serial.println(now.year());
    }
    vTaskDelete(NULL);
}


