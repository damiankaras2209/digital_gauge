#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include "Log.h"

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


class NetworkingClass {

    public:
        int connectWiFi(int = CONNECTING_TIME, const char* ssid = "", const char* pass = "");
        static void serverSetup();
        void sendEvent(const char *, std::string);
    private:
        static void f(std::string);
        [[noreturn]] static void WiFiGotIP(WiFiEvent_t, WiFiEventInfo_t);
        static void serverSetupTask(void *);
};

extern NetworkingClass Networking;

#endif