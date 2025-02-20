#ifndef _DATA_H
#define _DATA_H

#include <cstdio>
#include <cmath>
#include <sstream>
#include <iomanip>

#include "Log.h"
#include "Settings.h"
#include "WiFi.h"
#include "Lock.h"
#include "Date.h"

#include "RTClib.h"
#include "GxFT5436.h"
#include "ADS1X15.h"
#include "muParser.h"

#define SAMPLES_ADC 32

enum Device {
    D_FT5436, D_DS3231, D_MCP2515, D_ADS1115, D_A24C32, D_LAST
};

const String deviceName[] = {"FT5436", "DS3231", "MCP2515", "ADS1115", "AT24C32"};

enum Result {
    D_SUCCESS, D_FAIL
};

class DataClass {

public:

    typedef struct DataInput {
        volatile bool visible = false;
        volatile float voltage = 0.0f;
        volatile float resistance = 0.0f;
        volatile float value = 0.0f;
        std::string toString() {
            std::stringstream ss;
            ss << "[" << voltage << "," << resistance << "," << value << "]";
            return ss.str();
        }
    } DataInput;

    typedef struct DataStruct {
            RTC_DS3231* rtcPtr;
            ADS1115* adsPtr;

            DataInput *dataInput;
            DateTime now;
            ulong lastRTC = 0;
            Lock lock;
    } DataStruct;

        DataClass();
        void init();
        void POST();
        bool status[D_LAST];
        GxFT5436 touch = GxFT5436(21, 22,-1);
        RTC_DS3231 rtc;
        ADS1115 ads = ADS1115(0x48);

        DataInput* getDataInput();
        DateTime getTime();
        DataStruct data;
        DataInput dataInput[SettingsClass::LAST];

        typedef std::function<void(const char *, std::string )> SendEvent;
        typedef std::function<size_t()> CountClients;
        SendEvent _sendEvent = nullptr;
        CountClients _countClients = nullptr;
        void setEvent(SendEvent e);
        void setCountClients(CountClients e);

        static int adjustTime(DataStruct *);

private:
        static void readTime(DataStruct *dataStruct);
        _Noreturn  static void adcLoop(void *);

};

extern DataClass Data;

#endif
