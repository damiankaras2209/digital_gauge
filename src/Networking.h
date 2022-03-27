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

#define HOSTNAME "esp32"

class NetworkingClass {

    public:
        int connectWiFi(const char* ssid = "", const char* pass = "");
        void sendInfo();
        void serverSetup();
        void sendEvent(const char *event, std::string content);
        bool* getServerOnPointer() {return &_serverOn;};

    private:

        AsyncWebServer *server;
        AsyncEventSource *events;

        bool _serverOn = false;

        struct Credentials {
            const char *ssid = "";
            const char *pass = "";
        } _credentials;

        _Noreturn  static void connectionMaintainer(void *);
        static void serverSetupTask(void *);
        static String processor(const String& var);
};

extern NetworkingClass Networking;

#endif