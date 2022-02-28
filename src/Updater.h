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
//#include "Networking.h"

#define URL "http://217.11.128.153:1337/"

typedef struct Version {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint32_t toInt() const {
        return major << 16 | minor << 8 | patch;
    }
    String toString() const {
        return major + (String)"." + minor + (String)"." + patch;
    }
    bool operator == (const Version & v) const {
        return toInt() == v.toInt();
    }
    bool operator != (const Version & v) const {
        return toInt() != v.toInt();
    }
    bool operator > (const Version & v) const {
        return toInt() > v.toInt();
    }
    bool operator < (const Version & v) const {
        return toInt() < v.toInt();
    }

} Version;

typedef std::function<void(t_httpUpdate_return status)> Callback;
typedef std::function<void(String str)> LogCallback;
typedef std::function<void()> OnFinnishCallback;
typedef std::function<void()> OnSuccessCallback;

class UpdaterClass {

    public:
        Version firmware, filesystemCurrent, filesystemTarget;
        void init();
        void setOnFinnish(OnFinnishCallback);
        void setOnSuccessCallback(OnSuccessCallback);
        void checkForUpdate(LogCallback log = nullptr);
        void updateFS(String, Callback);
        void loop();
        String getMac();

    private:
        OnFinnishCallback _onFinnish = nullptr;
        OnSuccessCallback _onSuccess = nullptr;
        LogCallback _log = nullptr;
        volatile bool _check = false;
        static void readFilesystemVersion(Version*);
        void updateFW(String);

};

extern UpdaterClass Updater;

#endif
