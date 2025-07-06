#include "mWiFi.h"

MWiFiClass MWiFi;

bool MWiFiClass::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

int MWiFiClass::connectWiFi(const char* ssid, const char* pass) {

    if (WiFi.status() == WL_CONNECTED)
        return WiFi.status();


    _credentials.ssid = ssid;
    _credentials.pass = pass;

    WiFi.onEvent([this](WiFiEvent_t event, WiFiEventInfo_t info) {
        Log.logf("Connected to %s, IP: %s\n", WiFi.SSID().c_str(),IPAddress(info.wifi_ap_staipassigned.ip.addr).toString().c_str());

        Log.logf("Connected; total: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

//        if (!MDNS.begin(HOSTNAME)) { //http://esp32.local
//            Log.logf("Error setting up MDNS responder!");
//        }
//        Log.logf("mDNS responder started, hostname: http://%s.local\n", HOSTNAME);

//        Log.logf("MDNS; total: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

    if (onConnectedCallback != nullptr)
        onConnectedCallback();


    }, ARDUINO_EVENT_WIFI_STA_GOT_IP);

    WiFi.disconnect(true, true);
    WiFi.mode(WIFI_MODE_STA);
    // delay(2000);
    WiFi.begin(_credentials.ssid, _credentials.pass);

    TaskHandle_t handle;
    if (!xTaskCreate(connectionMaintainer,
                     "connectionMaintainer",
                     4 * 1024,
                     &_credentials,
                     1,
                     &handle))
        Log.logf("Failed to start connectionMaintainer task");


    Log.logf("Connecting to WiFi network: \"%s\"\n", ssid);
    return 0;
}

_Noreturn void MWiFiClass::connectionMaintainer(void * pvParameters) {
    for(;;) {
        while(WiFi.status() != WL_CONNECTED) {
            WiFi.reconnect();
//            Log.logf("Result: %d\n", WiFi.waitForConnectResult());
            delay(5000);
        }
        delay(1000);
    }
}