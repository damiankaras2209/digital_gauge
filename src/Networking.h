#ifndef _NETWORKING_H_
#define _NETWORKING_H_

#include "Log.h"
#include <Data.h>
#include <WebServer.h>

#define HOSTNAME "esp32"

class NetworkingClass {

    public:
        int connectWiFi(const char* ssid = "", const char* pass = "");

    private:
        struct Credentials {
            const char *ssid = "";
            const char *pass = "";
        } _credentials;

        _Noreturn  static void connectionMaintainer(void *);
};

extern NetworkingClass Networking;

#endif