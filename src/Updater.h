#ifndef _UPDATER_H
#define _UPDATER_H

#include "Log.h"

#include <stdlib.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "ArduinoJson.h"
#include "SPIFFS.h"
#include "HTTPUpdate.h"
#include "Settings.h"
#include <sstream>

uint32_t getCurrentFirmwareVersion();
String getCurrentFirmwareVersionString();

uint32_t getTargetFilesystemVersion();
String getTargetFilesystemVersionString();

uint32_t getCurrentFilesystemVersion();
String getCurrentFilesystemVersionString();


typedef std::function<void(t_httpUpdate_return status)> Callback;

class UpdaterClass {

    public:
        void checkForUpdate();
        void updateFS(String, Callback);

    private:

        void updateFW(String);

};

//UpdaterClass Updater;

#endif
