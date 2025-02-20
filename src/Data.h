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

#include "mcp2515.h"
#include "RTClib.h"
#include "GxFT5436.h"
#include "ADS1X15.h"
#include "Adafruit_MCP23X08.h"
#include "RCSwitch.h"
#include "muParser.h"

#define SAMPLES_ADC 32
#define SAMPLES_CAN 4

#define CAN_INACTIVITY_THRESHOLD 3500
#define CAN_REINIT_AFTER 1000

#define CAN_ID_STEERING_ANGLE 0x80
#define CAN_ID_RPM_SPEED_GAS 0x201
#define CAN_ID_AC 0x440
#define CAN_ID_HB 0x430

#define THROTTLE_VALVE_POWER_ON_TIME 2500
#define THROTTLE_VALVE_DELAY 1000

#define HEADLIGHTS_PIN 6
#define THROTTLE_PIN 5
#define THROTTLE_POWER_PIN 4


//typedef struct Event {
//    uint16_t event;
//    uint16_t beginX[5], beginY[5];
//    uint16_t endX[5], endY[5];
//} Event;
//
//typedef std::function<void(t_httpUpdate_return status)> Callback;

enum Device {
    D_FT5436, D_DS3231, D_MCP2515, D_ADS1115, D_MCP23008, D_A24C32, D_LAST
};

const String deviceName[] = {"FT5436", "DS3231", "MCP2515", "ADS1115", "MCP23008", "AT24C32"};

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
            MCP2515* mcp2515Ptr;
            ADS1115* adsPtr;
            Adafruit_MCP23X08* mcp23X08Ptr;
            RCSwitch* rcPtr;

            DataInput *dataInput;
            DateTime now;
            ulong lastRTC = 0;
            ulong lastFrame = 0;
            ulong lastCanInit = 0;
            bool canActive = false;
            bool shouldToggleValve = false;
            ulong lastValveChange = 0;
            ulong lastValvePowerOn = 0;
            bool relayState[8] = {LOW, LOW, LOW, LOW, LOW, LOW, LOW, LOW};
            Lock lock;
    } DataStruct;

        DataClass();
        void init();
        void POST();
        bool status[D_LAST];
        GxFT5436 touch = GxFT5436(21, 22,-1);
        RTC_DS3231 rtc;
        MCP2515 mcp = MCP2515(5);
        ADS1115 ads = ADS1115(0x48);
        Adafruit_MCP23X08 mcp23008;
        RCSwitch rc;

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
//        static void IRAM_ATTR touchStart();

//        [[noreturn]] static void test(void *);
        static void readTime(DataStruct *dataStruct);
        static void canReset(MCP2515* mcp);
        _Noreturn  static void adcLoop(void *);
        _Noreturn  static void canLoop(void *);

};

extern DataClass Data;

#endif
