#ifndef _MWIFI_H
#define _MWIFI_H

#include "Log.h"
#include <WiFi.h>

#define HOSTNAME "esp32"

class MWiFiClass {

    friend class NetworkingClass;

    private:
        int connectWiFi(const char* ssid = "", const char* pass = "");
        bool isConnected();
        std::function<void()> onConnectedCallback = nullptr;

        struct Credentials {
            const char *ssid = "";
            const char *pass = "";
        } _credentials;

        _Noreturn  static void connectionMaintainer(void *);

};

extern MWiFiClass MWiFi;


#endif
