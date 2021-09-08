#ifndef _DATA_H
#define _DATA_H

#include <cstdio>

#include "mcp2515.h"
#include "RTClib.h"
#include "TFT_eSPI.h"
#include "GxFT5436.h"
#include "ADS1X15.h"

#include "time.h"
#include "WiFi.h"

class Data {

public:
    typedef struct DataStruct {
        ADS1115* adsPtr;
        RTC_DS3231* rtcPtr;
        GxFT5436* touchPtr;
        float adc[4] = {0, 0, 0, 0};
        DateTime now;
        int brightness = 255;
    } DataStruct;

        Data();
        void init();
        RTC_DS3231 rtc;
        MCP2515 mcp = MCP2515(5);
        ADS1115 ads = ADS1115(0x48);
        GxFT5436 touch = GxFT5436(/*SDA=*/21, /*SCL=*/22,/*RST=*/0);
        DateTime getTime();
        DataStruct data;

private:
        static void IRAM_ATTR touchStart();

    [[noreturn]] static void test(void *);
        static boolean RTCAvailable;
        static void adjustRTCTask(void *);
        _Noreturn  static void adcLoop(void *);

};


#endif
