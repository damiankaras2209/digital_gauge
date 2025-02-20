#include <esp_system.h>

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


     Networking.connectWiFi((char *)Settings.general[WIFI_SSID]->getString().c_str(), (char *)Settings.general[WIFI_PASS]->getString().c_str());

     Data.POST();
     Data.init();

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

unsigned  long t15;
void loop() {

    t15 = millis();

    Screen.tick();

    std::stringstream ss;
    ss << millis()-t15;
    Networking.sendEvent("frametime", ss.str(), millis());
}