#include <esp_system.h>
#include <Updater.h>

#include "Networking.h"
#include "Data.h"
#include "Settings.h"
#include "Screen.h"

void setup() {
    Serial.begin(115200);

    while (!Serial)

    Log.logf("Hello");


    if (!SPIFFS.begin()) {
        Log.logf("SPIFFS initialisation failed!");
    }

    Settings.init();
    S_STATUS settingsStatus = Settings.load();
    Settings.loadState();

    Screen.init();

    Updater.init();

    Log.logf("Firmware version: %s(%d)\n", Updater.firmware.toString().c_str(), Updater.firmware.toInt());
    Log.logf("Filesystem version: %s(%d), target: %s(%d)\n", Updater.filesystemCurrent.toString().c_str(), Updater.filesystemCurrent.toInt(),  Updater.filesystemTarget.toString().c_str(), Updater.filesystemTarget.toInt());
    if(Updater.filesystemTarget != Updater.filesystemCurrent) {
        Screen.setBrightness(255);
        Log.logf("Filesystem version does not match target version. Trying to update");

        Screen.showPrompt("Filesystem version does not match target version\ncurrent: " +
                          Updater.filesystemCurrent.toString() +
                          ", target: " +
                          Updater.filesystemTarget.toString() +
                          "\nCreate an AP with following credentials:\nSSID: \"" +
                          (char *)Settings.general[WIFI_SSID]->getString().c_str() +
                          "\", pass: \"" +
                          (char *)Settings.general[WIFI_PASS]->getString().c_str() +
                          "\"",
        4, true);
        Screen.tick();

        Networking.connectWiFi((char *)Settings.general[WIFI_SSID]->getString().c_str(), (char *)Settings.general[WIFI_PASS]->getString().c_str());
        while(WiFi.status() != WL_CONNECTED){
            delay(50);
        }
        Screen.appendToPrompt("\nWiFi connected, updating... this may take a while");
        Screen.tick();
        Updater.updateFS(Updater.filesystemTarget.toString(), [](t_httpUpdate_return status) {
            switch (status) {
                case HTTP_UPDATE_FAILED:
                    Log.logf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                    Screen.appendToPrompt("\nUpdate failed " + (String) httpUpdate.getLastError() + ": " +
                        httpUpdate.getLastErrorString().c_str() + "\nReboot to try again");
                    Screen.tick();
                    break;
                case HTTP_UPDATE_NO_UPDATES:
                    Log.logf("HTTP_UPDATE_NO_UPDATES");
                    break;

                case HTTP_UPDATE_OK:
                    Log.logf("HTTP_UPDATE_OK");
                    Settings.save();
                    Screen.appendToPrompt("\nUpdate successful\n Restarting in 3 seconds");
                    Screen.tick();
                    delay(3000);
                    esp_restart();
            }
        });
    } else {
        Networking.connectWiFi((char *)Settings.general[WIFI_SSID]->getString().c_str(), (char *)Settings.general[WIFI_PASS]->getString().c_str());

        Data.POST();
        Data.init();
    }

    Screen.switchView(GAUGES);

    switch (settingsStatus) {
        case S_FAIL: {
            Settings.save();
            Screen.showPrompt("Unable to load settings\nUsing default");
            break;
        }
        case S_MISSING: {
            Settings.save();
            Screen.showPrompt("Some settings are missing\nThose have been set to default");
        }
    }

    Screen.setBrightness(255);

  Log.logf("setup() complete");
}

unsigned  long t15, t16 = 0;
int f;
void loop() {

    t15 = millis();

    Screen.tick();

    f++;

    if (millis() > t16 + 1000) {
        const int fps = lround(static_cast<double>(f*1000)/(millis()-t16));
        Log.logf("FPS: %d\n", fps);
        t16 = millis();
        f = 0;
    }

    Updater.loop();
}