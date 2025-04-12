#include "Networking.h"

#include <utility>

NetworkingClass Networking;

// bool NetworkingClass::isBLEConnected() {
//     return BLE.isConnected();
// }

bool NetworkingClass::isWiFiConnected() {
    return MWiFi.isConnected();
}

void NetworkingClass::connectWiFi(const char* ssid, const char* pass) {
    MWiFi.connectWiFi(ssid, pass);
}

// void NetworkingClass::connectBLE() {
//     BLE.startBLEServer();
// }
//
// void NetworkingClass::sendMessage(const std::string& msg) {
//     BLE.sendMessage(msg);
// }

void NetworkingClass::setOnWiFiConnectedCallback(std::function<void()> f) {
    MWiFi.onConnectedCallback = std::move(f);
}
