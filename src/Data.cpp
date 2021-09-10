#include "Data.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

static Data::DataStruct data;
volatile bool i2cBusy = false;
volatile ulong touchDetectedTime = 0;

volatile ulong time2 = 0;

[[noreturn]] void Data::test(void * pvParameters) {
    Serial.print(pcTaskGetTaskName(NULL));
    Serial.print(" started on core ");
    Serial.println(xPortGetCoreID());
    while(1) {
//        Serial.print("millis() - touchDetectedTime: ");
//        Serial.println (millis() - touchDetectedTime);
        if(millis() - touchDetectedTime < 10) {
            while(i2cBusy)
                delay(1);
            i2cBusy = true;

            GxFT5436* touch = ((DataStruct*)pvParameters)->touchPtr;
            GxFT5436::TouchInfo touchInfo = touch->scanMultipleTouch();

            for (uint8_t i = 0; i < touchInfo.touch_count; i++) {

                uint16_t x1 = touchInfo.x[i];
                touchInfo.x[i] = touchInfo.y[i];
                touchInfo.y[i] = 319-x1;
//                        Serial.print("gesture: ");
//                        Serial.print(touchinfo.gesture);
//                    transpose(touchinfo);
                //                if(touchinfo.y[i] > TFT_WIDTH/2) {
                //                    *brightness += 25;
                //                    if(*brightness>255)
                //                        *brightness=255;
                //                } else {
                //                    *brightness -= 25;
                //                    if(*brightness<0)
                //                        *brightness=0;
                //                }
                //                Serial.print("brightness: ");
                //                Serial.println(*brightness);
                //                ledcWrite(0, *brightness);
                Serial.print("touch id: ");
                Serial.print(touchInfo.id[i]);
                Serial.print(", event: ");
                Serial.print(touchInfo.event[i]);
                Serial.print(" (");
                Serial.print(touchInfo.x[i]);
                Serial.print(", ");
                Serial.print(touchInfo.y[i]);
                Serial.print(") ");
            }
            Serial.println("");

            i2cBusy = false;
        }
        delay(7);
    }
}

void IRAM_ATTR Data::touchStart() {
    touchDetectedTime = millis();
}

Data::Data() {

}


void Data::init() {
    HardwareSerial& DiagnosticStream = Serial;
    touch.init(&Serial);

    data = DataStruct();
    data.adsPtr = &ads;
    data.rtcPtr = &rtc;
    data.touchPtr = &touch;

    if (!rtc.begin())
        Serial.println("Couldn't find RTC");
    if(rtc.lostPower()) {
        Serial.println("Lost power");
        rtc.adjust(DateTime(1, 1, 1, 0, 0, 0));
    }

    //    TaskHandle_t handle1;
    //    Serial.println(
    //            xTaskCreatePinnedToCore(adjustRTCTask,
    //                                    "adjustRTC",
    //                                    32*1024,
    //                                    &data,
    //                                    3,
    //                                    &handle1,
    //                                    1));



    //    mcp.reset();
    //    mcp.setBitrate(CAN_500KBPS, MCP_8MHZ);
    //    mcp.setListenOnlyMode();




    ads.begin();
    ads.setGain(1);
    ads.setDataRate(7);


    pinMode(33, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(33), touchStart, RISING);
    //    attachInterrupt(digitalPinToInterrupt(33), touchEnd, FALLING);


    TaskHandle_t touchHandle;
    //    Serial.print(" touch start: ");
//    Serial.println(
//            xTaskCreatePinnedToCore(test,
//                                    "touch",
//                                    4*1024,
//                                    &data,
//                                    1,
//                                    &touchHandle, 0) ? "" : "Failed to start touch task");

    TaskHandle_t handle2;
    Serial.print("adcLoop task: ");
    Serial.println(
            xTaskCreatePinnedToCore(adcLoop,
                "adcLoop",
                4*1024,
                &data,
                1,
                &handle2,
                0) ? "" : "Failed to start adcLoop task");

}



_Noreturn void Data::adcLoop(void * pvParameters) {
    Serial.print(pcTaskGetTaskName(NULL));
    Serial.print(" started on core ");
    Serial.println(xPortGetCoreID());

    DataStruct *params = (DataStruct*)pvParameters;

    for(;;) {
        while(i2cBusy)
           delay(1);
        i2cBusy = true;
//        Serial.print(((DataStruct*)pvParameters)->adsPtr->getMaxVoltage());
//        Serial.print(", ");
        for(int i=0; i<4; i++) {

            for(int j=SAMPLES; j>0; j--) {
                params->adc[i][j] = params->adc[i][j-1];
                params->adc[i][0] = params->adsPtr->readADC(i);
            }
            float sum = 0;
            for(int j=0; j<SAMPLES; j++) {
                Serial.print(params->adc[i][j]);
                Serial.print(", ");
                sum += params->adc[i][j];
            }
            Serial.println("");
            params->adcVoltage[i] = params->adsPtr->toVoltage(sum/SAMPLES);

            //            Serial.println("looooop");
//            Serial.print(i);w
//            Serial.print(": ");
//            Serial.print(((DataStruct*)pvParameters)->adc[i]);
//            Serial.print(" ");
        }
//        Serial.println(" ");
//        test();
        params->now = params->rtcPtr->now();
//        test(params->touchPtr, &(params->brightness));

        i2cBusy = false;
        delay(10);
    }
    vTaskDelete(NULL);
}

//_Noreturn void rpmRead(void * pvParameters) {
//    while(true) {
//
//        struct can_frame frame;
//        frame.can_id = 0x213;
//        frame.can_dlc = 4;
//        frame.data[0] = 0xFF;
//        frame.data[1] = 0xFF;
//        frame.data[2] = 0xFF;
//        frame.data[3] = 0xF0;
//        //  mcp2515.sendMessage(&frame);
//
//        MCP2515::ERROR err = mcp.readMessage(&canMsg);
//
//        if (err == MCP2515::ERROR_OK && canMsg.can_id == 0x201) {
//            Serial.print(canMsg.can_id, HEX); // print ID
//            Serial.print(" ");
//            Serial.print(canMsg.can_dlc, HEX); // print DLC
//            Serial.print(" ");
//
//            for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
//                Serial.print(canMsg.data[i],HEX);
//                Serial.print(" ");
//            }
//
//            rpm[2] = rpm[1];
//            rpm[1] = rpm[0];
//            rpm[0] = (canMsg.data[0]*256 + canMsg.data[1])/4;
//
//
//            Serial.print("RPM: ");
//            Serial.print((rpm[0]+rpm[1]+rpm[2])/3);
//
//            Serial.println();
//            rpmVal = (rpm[0]+rpm[1]+rpm[2])/3;
//        } else {
//            // return rpmVal;
//            //    if( err != MCP2515::ERROR_NOMSG) {
//            //     Serial.print("Error: ");
//            //     Serial.print(err);
//            //     Serial.print("\n");
//            //    }
//        }
//        delay(10);
//    }
//}


DateTime Data::getTime() {
    return data.now;
}

void Data::adjustRTCTask(void * pvParameters) {
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
//        DateTime now  = RTC_DS3231::now();
//        Serial.println("min: ");
//        Serial.println(now.minute());
    }
    vTaskDelete(NULL);
}


