#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include "ESPAsyncWebServer.h"
#include "Log.h"
#include "Screen.h"

#define HOSTNAME "esp32"

class NetworkingClass {

    public:
        int connectWiFi(const char* ssid = "", const char* pass = "");
        void sendInfo();
        void serverSetup();
        void sendEvent(const char *event, const std::string& content, ulong id) const;
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
        static String processorRoot(const String& var);
        static String processorLog(const String& var);
        static String processorOta(const String& var);
};

extern NetworkingClass Networking;

#endif