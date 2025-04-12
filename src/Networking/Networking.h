#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include <string>
#include "mWiFi.h"
// #include "BLE.h"

class NetworkingClass {

    public:
        enum NetworkType {
            NETWORK_TYPE_WIFI, NETWORK_TYPE_BLE, NETWORK_TYPE_LAST
        };

        const std::string networkTypeNme[2] = {"WiFi", "BLE"};

        // bool isBLEConnected();
        bool isWiFiConnected();
        // void connectBLE();
        void connectWiFi(const char* ssid = "", const char* pass = "");
        // void sendMessage(const std::string& msg);

        void setOnWiFiConnectedCallback(std::function<void()>);
};

extern NetworkingClass Networking;

#endif