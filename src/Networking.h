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

#define HOSTNAME "esp32"

class NetworkingClass {

    private:

        static void f(std::string);
        static void serverSetupTask(void *);

    public:
        int connectWiFi(const char* ssid = "", const char* pass = "");
        void sendInfo();
        static void serverSetup();
        void sendEvent(const char *event, std::string content);

    private:
        struct Credentials {
            const char *ssid = "";
            const char *pass = "";
        } _credentials;
        _Noreturn  static void connectionMaintainer(void *);
};

extern NetworkingClass Networking;

#endif