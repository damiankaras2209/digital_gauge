#include "Data.h"

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;


Data::Data() {

}


void Data::init() {

    data = DataStruct();
    data.adsPtr = &ads;
    data.rtcPtr = &rtc;
    data.touchPtr = &touch;
    data.mcp23X08Ptr = &mcp23008;
    data.mcp2515Ptr = &mcp;
    data.rcPtr = &rc;

    if (!rtc.begin())
        Serial.println("Couldn't find RTC");
    if(rtc.lostPower()) {
        Serial.println("Lost power");
        rtc.adjust(DateTime(1, 1, 1, 22, 22, 22));
    }

    TaskHandle_t rtcAdjustHandle;
    Serial.println(xTaskCreatePinnedToCore(adjustRTCTask,
                                    "adjustRTC",
                                    4*1024,
                                    &data,
                                    3,
                                    &rtcAdjustHandle,
                                    0) ? "" : "Failed to start rtc adjust task");



    mcp.reset();
    mcp.setBitrate(CAN_500KBPS, MCP_8MHZ);
    mcp.setListenOnlyMode();


    ads.begin();
    ads.setGain(1);
    ads.setDataRate(7);


    touch.init(&Serial);

    TaskHandle_t adcHandle;
    Serial.println(xTaskCreatePinnedToCore(adcLoop,
                                    "adcLoop",
                                    4*1024,
                                    &data,
                                    1,
                                    &adcHandle,
                                    0) ? "" : "Failed to start adcLoop task");


    TaskHandle_t canHandle;
    Serial.println( xTaskCreatePinnedToCore(canLoop,
                                            "can",
                                            32*1024,
                                            &data,
                                            1,
                                            &canHandle, 0) ? "" : "Failed to start can task");

    rc.enableReceive(3);

}



_Noreturn void Data::adcLoop(void * pvParameters) {
    Serial.print(pcTaskGetTaskName(NULL));
    Serial.print(" started on core ");
    Serial.println(xPortGetCoreID());

    DataStruct *params = (DataStruct*)pvParameters;

    for(;;) {
        while(params->i2cBusy)
           delay(1);
        params->i2cBusy = true;

        for(int i=0; i<4; i++) {

            for(int j=SAMPLES-1; j>0; j--)
                params->adc[i][j] = params->adc[i][j-1];
            params->adc[i][0] = params->adsPtr->readADC(i);
            float sum = 0;
//            Serial.print(i);
//            Serial.print(" ");
            for(int j=0; j<SAMPLES; j++) {
//                Serial.print(params->adc[i][j]);
//                Serial.print(", ");
                sum += params->adc[i][j];
            }
//            Serial.println("");
            params->adcVoltage[i] = params->adsPtr->toVoltage(sum/SAMPLES);
        }

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


