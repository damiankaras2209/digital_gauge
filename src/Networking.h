#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include <WiFi.h>
#include <WiFiClient.h>
#include "ESPAsyncWebServer.h"
#include <ESPmDNS.h>
#include "TFT_eSPI.h"
#include <string>

#include "Screen.h"
#include "Settings.h"
#include "Updater.h"

#define CONNECTING_TIME 30*1000
#define TIME_NO_WAIT 0
#define TIME_INFINITY -1

class Networking {
    public:
        int connectWiFi(int = CONNECTING_TIME, const char* ssid = "", const char* pass = "");
        static void serverSetup();
        static boolean isWiFiConnected();
    private:
        [[noreturn]] static void WiFiStationConnected(WiFiEvent_t, WiFiEventInfo_t);
        [[noreturn]] static void WiFiGotIP(WiFiEvent_t, WiFiEventInfo_t);
        static void connectWiFiTask(void *);
        static void serverSetupTask(void *);
};

#endif