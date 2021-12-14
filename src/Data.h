#ifndef _DATA_H
#define _DATA_H

#include "Log.h"

#include <cstdio>

#include "mcp2515.h"
#include "RTClib.h"
#include "TFT_eSPI.h"
#include "GxFT5436.h"
#include "ADS1X15.h"
#include <Adafruit_MCP23X08.h>
#include "RCSwitch.h"

#include "Settings.h"
#include "time.h"
#include "WiFi.h"

#include <numeric>

#include <sstream>
#include <iomanip>

#define SAMPLES_ADC 32
#define SAMPLES_CAN 4

#define CAN_ID_STEERING_ANGLE 0x80
#define CAN_ID_RPM_SPEED_GAS 0x201
#define CAN_ID_AC 0x440
#define CAN_ID_HB 0x430

//typedef struct Event {
//    uint16_t event;
//    uint16_t beginX[5], beginY[5];
//    uint16_t endX[5], endY[5];
//} Event;
//
//typedef std::function<void(t_httpUpdate_return status)> Callback;

class Data {

public:

    typedef struct DataStruct {
            RTC_DS3231* rtcPtr;
            MCP2515* mcp2515Ptr;
            class ADS1115* adsPtr;
            GxFT5436* touchPtr;
            Adafruit_MCP23X08* mcp23X08Ptr;
            RCSwitch* rcPtr;

            bool GxFT5436Available;
            bool RTCAvailable;
            GxFT5436::TouchInfo touchInfo;
            DateTime now;
            int brightness = 255;
            ulong lastRTC = 0;
            volatile bool i2cBusy = false;
        } DataStruct;

        Data();
        void init();
        RTC_DS3231 rtc;
        MCP2515 mcp = MCP2515(5);
        ADS1115 ads = ADS1115(0x48);
        GxFT5436 touch = GxFT5436(/*SDA=*/21, /*SCL=*/22,/*RST=*/-1);
        Adafruit_MCP23X08 mcp23008;
        RCSwitch rc;

        DateTime getTime();
        DataStruct data;

private:
//        static void IRAM_ATTR touchStart();

//        [[noreturn]] static void test(void *);
        static void adjustRTCTask(void *);
        _Noreturn  static void adcLoop(void *);
        _Noreturn  static void canLoop(void *);

};


#endif
