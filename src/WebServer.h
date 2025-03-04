#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include "ESPAsyncWebServer.h"
#include "Log.h"
#include <Screen.h>

class WebServerClass {
    public:
        WebServerClass() = default;
        void sendInfo();
        void serverSetup();
        void sendEvent(const char *event, const std::string& content, ulong id) const;
        bool* getServerOnPointer() {return &_serverOn;};

    private:

        AsyncWebServer *server;
        AsyncEventSource *events;

        bool _serverOn = false;

        static void serverSetupTask(void *);
        static String processorRoot(const String& var);
        static String processorScript(const String& var);
        static String processorLog(const String& var);
        static String processorOta(const String& var);
};


extern WebServerClass WebServer;


#endif //_WEBSERVER_H
