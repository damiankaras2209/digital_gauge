#include "Updater.h"
#include "Cert.h"

const char* url = "https://firebasestorage.googleapis.com/v0/b/gauge-7c5b4.appspot.com/o/";

const char* FIRMWARE = "firmware";
const uint8_t version_fw[] = {1, 0, 0};

const char* FILESYSTEM = "spiffs";
const uint8_t target_fs[] = {1, 0, 0};


uint32_t getCurrentFirmwareVersion() {
    return version_fw[0] << 16 | version_fw[1] << 8 | version_fw[2];
}

String getCurrentFirmwareVersionString() {
    return version_fw[0] + (String)"." + version_fw[1] + (String)"." + version_fw[2];
}

uint32_t getTargetFilesystemVersion() {
    return target_fs[0] << 16 | target_fs[1] << 8 | target_fs[2];
}

String getTargetFilesystemVersionString() {
    return target_fs[0] + (String)"." + target_fs[1] + (String)"." + target_fs[2];
}

uint32_t getCurrentFilesystemVersion() {
    if(SPIFFS.exists("/version.txt")) {

        fs::File file = SPIFFS.open("/version.txt", "r");
        uint8_t buf[20];
        file.read(buf,20);
        char* line = (char *)malloc(20*sizeof(char));
        for(int i=0; i<20; i++) {
            if(buf[i] != 13/*CR*/) {
                line[i] = (char)buf[i];
//                Serial.print(" ");
//                Serial.print(buf[i]);
            } else {
                line[i] = '\0';
                break;
            }
        }
//        Serial.println("");
//        Serial.print("Line: ");
//        Serial.println(line);

        uint8_t fs_current[] = {0, 0, 0};

        char* p = line;

        fs_current[0] = std::strtol(p, &p, 10);
        fs_current[1] = std::strtol(p+1, &p, 10);
        fs_current[2] = std::strtol(p+1, &p, 10);
        free(line);

        return fs_current[0] << 16 | fs_current[1] << 8 | fs_current[2];

    } else
        return 0;
}

String getCurrentFilesystemVersionString() {
    uint32_t version = getCurrentFilesystemVersion();
    return (String)(version >> 16) + "." + (String)((version >> 8) & 0xff) + "." + (String)(version & 0xff);
}

void UpdaterClass::updateFW(String url) {
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);
    Serial.println(url);
    t_httpUpdate_return ret = httpUpdate.update(client, url);

    switch (ret) {
        case HTTP_UPDATE_FAILED:
            Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;

            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("HTTP_UPDATE_NO_UPDATES");
                break;

                case HTTP_UPDATE_OK:
                    Serial.println("HTTP_UPDATE_OK");
                    break;
    }


}

void UpdaterClass::updateFS(String version, Callback onFinish) {
    WiFiClientSecure client;
    client.setCACert(rootCACertificate);

    std::stringstream ss;

    ss << url;
    ss << FILESYSTEM;
    ss << "_v";
    ss << version.c_str();
    ss << ".bin?alt=media";


    Serial.println(ss.str().c_str());

    t_httpUpdate_return ret = httpUpdate.updateSpiffs(client, ss.str().c_str());

    onFinish(ret);
}


void UpdaterClass::checkForUpdate() {

    while (WiFi.status() != WL_CONNECTED) {
        delay(50);
//        Serial.print(".");
    }


    uint32_t fw_current = getCurrentFirmwareVersion();
    uint32_t fs_current = getCurrentFilesystemVersion();
    uint32_t fs_target = getTargetFilesystemVersion();


    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();

        DynamicJsonDocument doc(8*1024);
        deserializeJson(doc, payload);
//
//        String s = doc[0]["name"];
//        int size = doc.size();
//        Serial.println(s.c_str());

        JsonArray arr = doc["items"].as<JsonArray>();
        for (JsonVariant value : arr) {
            std::string filename = value["name"];

            size_t start = filename.find_last_of("_v") + 1;
            size_t end = filename.find_last_of('.');

            std::string name = filename.substr(0, start - 2);
            std::string version = filename.substr(start, end-start).c_str();
            char* p = const_cast<char *> (version.c_str());

            uint8_t ver[] = {0, 0, 0};

            ver[0] = std::strtol(p, &p, 10);
            ver[1] = std::strtol(p+1, &p, 10);
            ver[2] = std::strtol(p+1, &p, 10);

            Serial.print(filename.c_str());
            Serial.print(" ");
            Serial.print(name.c_str());
            for(uint8_t i=0; i<3; i++) {
                Serial.print(" ");
                Serial.print(ver[i]);
            }
            Serial.println("");


            uint32_t found = ver[0] << 16 | ver[1] << 8 | ver[2];
            if(name == FIRMWARE) {
                Serial.print("Firmware current version: ");
                Serial.print(fw_current);
                Serial.print(", found: ");
                Serial.println(found);
                if(found > fw_current) {
                    Serial.println("New firmware version found!");

                    std::stringstream ss;

                    ss << url;
                    ss << filename;
                    ss << "?alt=media";

                    updateFW(ss.str().c_str());
//                     updateFW(value["download_url"]);
                    break;
                } else {
                    Serial.println("Firmware is up to date!");
                }
//            } else if(name == FILESYSTEM) {
//                Serial.print("Filesystem current version: ");
//                Serial.print(fs_current);
//                Serial.print(", target: ");
//                Serial.print(fs_target);
//                Serial.print(", found: ");
//                Serial.println(found);
//                if(found == fs_target) {
//                    Serial.println("New filesystem  found!");
////                    updateFS(value["download_url"]);
//                    break;
//                } else {
//                    Serial.println("Found filesystem does not match target");
//                }
            }


//            uint8_t major = version.at(0) - 48;
//            uint8_t minor = version.at(2) - 48;
//            uint8_t patch = version.at(4) - 48;
//            Serial.println(str.c_str());
//            Serial.print(name.c_str());
//            Serial.print(" ");
        }


//        Serial.println(size);
//        Serial.println(str);
    } else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();

}