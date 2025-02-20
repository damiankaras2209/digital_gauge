#include "Updater.h"

UpdaterClass Updater;

const char* FIRMWARE_FILENAME = "firmware";
const char* FILESYSTEM_FILENAME = "spiffs";

void UpdaterClass::init() {
    firmware = Version{FIRMWARE};
    filesystemTarget = Version{FILESYSTEM};
    readFilesystemVersion(&filesystemCurrent);
    httpUpdate.rebootOnUpdate(false);
}

void UpdaterClass::setOnFinnish(OnFinnishCallback f) {
    _onFinnish = std::move(f);
}

void UpdaterClass::setOnSuccessCallback(OnSuccessCallback f) {
    _onSuccess = std::move(f);
}

void UpdaterClass::readFilesystemVersion(Version *v) {
    if(SPIFFS.exists("/version")) {

        fs::File file = SPIFFS.open("/version", "r");
        uint8_t buf[20];
        file.read(buf,20);
        char* line = (char *)malloc(20*sizeof(char));
        for(int i=0; i<20; i++) {
            if(buf[i] != 13/*CR*/) {
                line[i] = (char)buf[i];
                //                Log.logf(" ");
                //                Log.logf(buf[i]);
            } else {
                line[i] = '\0';
                break;
            }
        }
        //        Log.logf("");
        //        Log.logf("Line: ");
        //        Log.logf(line);

        char* p = line;

        v->major = std::strtol(p, &p, 10);
        v->minor = std::strtol(p+1, &p, 10);
        v->patch = std::strtol(p+1, &p, 10);
        free(line);

    } else {
        Log.logf("Version file does not exist");
    }
}

void UpdaterClass::performFWUpdate(String url) {
    WiFiClient client;
    Log.logf("Updating to: %s\n", url.c_str());
    Log.logf("Free heap: %d\n", ESP.getFreeHeap());
    t_httpUpdate_return ret = httpUpdate.update(client, url);

    std::stringstream ss;

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            ss << "HTTP_UPDATE_FAILD Error (" << httpUpdate.getLastError() << "): " << httpUpdate.getLastErrorString().c_str();
            break;
        case HTTP_UPDATE_NO_UPDATES:
            ss << "HTTP_UPDATE_NO_UPDATES";
            break;
        case HTTP_UPDATE_OK:
            ss << "Success";
            break;
    }
    Log.logf(ss.str().c_str());
    _log("\n");
    _log(ss.str().c_str());
    if(ret == HTTP_UPDATE_OK)
        if(_onSuccess != nullptr)
            _onSuccess();
}

void UpdaterClass::updateFS(String version, Callback onFinish) {
    WiFiClient client;

    std::stringstream ss;

    ss << URL;
    ss << "files/";
    ss << FILESYSTEM_FILENAME;
    ss << "_v";
    ss << version.c_str();
    ss << ".bin";

    Log.logf("SPIFFS url: %s\n", ss.str().c_str());

    t_httpUpdate_return ret = httpUpdate.updateSpiffs(client, ss.str().c_str());

    onFinish(ret);
}


void UpdaterClass::checkForUpdate(LogCallback log) {
    _log = std::move(log);
    _check = true;
}

void UpdaterClass::updateFW(String url) {
    _performUpdate = true;
    targetVersionURL = std::move(url);
}

void UpdaterClass::loop() {
    if (_performUpdate) {
        performFWUpdate(targetVersionURL);
    }
    if(_check) {

        HTTPClient http;
        http.begin(URL);
        int httpResponseCode = http.GET();

        if (httpResponseCode>0) {
            Log.logf("HTTP Response code: %d\n", httpResponseCode);
            std::string payload = http.getString().c_str();


            std::string latestFilename;
            Version latestFound = Version{0,0,0};
            Log.logf("Firmware current version: %s(%d)\n", firmware.toString().c_str(), firmware.toInt());

            std::size_t nextLine = 0;

            while(nextLine != std::string::npos) {
                nextLine = payload.find_first_of('\n');

                std::string line = payload.substr(0, nextLine);
                payload = payload.substr(nextLine+1);

//                Log.logf("Line length: %d\n", line.length());

                if(line.length() == 0)
                    continue;

                std::string filename = line.substr(line.find_last_of('/') + 1);

                size_t start = filename.find_last_of("_v") + 1;
                size_t end = filename.find_last_of('.');

                std::string name = filename.substr(0, start - 2);
                std::string version = filename.substr(start, end-start).c_str();
                char* p = const_cast<char *> (version.c_str());

                Version found;
                found.major = std::strtol(p, &p, 10);
                found.minor = std::strtol(p + 1, &p, 10);
                found.patch = std::strtol(p + 1, &p, 10);

                Log.logf("Found file: %s %s %d.%d.%d(%d)\n", filename.c_str(), name.c_str(), found.major, found.minor, found.patch, found.toInt());

                if(name == FIRMWARE_FILENAME) {
                    if(found > latestFound) {
                        latestFilename = filename;
                        latestFound = found;
                    }
                }
            }

            if(latestFound > firmware) {
                std::stringstream ss;
                ss << URL;
                ss << "files/";
                ss << latestFilename;
                _log("\nUpdating to " + latestFound.toString());
                _log("\nThis may take a few minutes...");
                performFWUpdate(ss.str().c_str());
            } else if (latestFound == firmware){
                Log.logf("Firmware is up to date!");
                _log("\nFirmware is up to date!");
            } else {
                Log.logf("No files found");
                _log("\nNo files found");
            }
        } else {
            Log.logf("Error code: %d\n", httpResponseCode);
            _log("\nError code: " + (String)httpResponseCode);
        }
        http.end();
        if(_onFinnish != nullptr)
            _onFinnish();
        _check = false;
    }
}

String UpdaterClass::getMac() {
    uint64_t chipid = ESP.getEfuseMac();
    uint16_t chip = (uint16_t)(chipid >> 32);
    char mac[13];
    snprintf(mac, 13, "%04X%08X", chip, (uint32_t)chipid);
    return mac;
}
