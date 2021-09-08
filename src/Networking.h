#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include <WiFi.h>
#include <WiFiClient.h>
#include "ESPAsyncWebServer.h"
#include <ESPmDNS.h>
#include <Update.h>
#include "TFT_eSPI.h"
#include <string>

//#include "settingsIndex.h"

#include "Screen.h"
#include "Settings.h"


class Networking {
    public:
        void connectWiFi();
        void serverSetup();
        static boolean isWiFiConnected();
    private:
        static void connectWiFiTask(void *);
        static void serverSetupTask(void *);
};

#endif