#include <Wire.h>
#include <SPI.h>
#include <esp_system.h>

#include "Networking.h"
#include "Data.h"
#include "Settings.h"
#include "Screen.h"
#include "Updater.h"

#include <WiFi.h>
#include "time.h"

#include "TFT_eSPI.h"

bool proceed = true;

void print_reset_reason(esp_reset_reason_t reason)
{
    switch(reason)
    {
        case 1 : Serial.println ("ESP_RST_UNKNOWN"); break;        //!< Reset reason can not be determined
        case 3 : Serial.println ("ESP_RST_POWERON"); break;        //!< Reset due to power-on event
        case 4 : Serial.println ("ESP_RST_EXT"); break;            //!< Reset by external pin (not applicable for ESP32)
        case 5 : Serial.println ("ESP_RST_SW"); break;             //!< Software reloadSettings via esp_restart
        case 6 : Serial.println ("ESP_RST_PANIC"); break;          //!< Software reloadSettings due to exception/panic
        case 7 : Serial.println ("ESP_RST_INT_WDT"); break;        //!< Reset (software or hardware) due to interrupt watchdog
        case 8 : Serial.println ("ESP_RST_TASK_WDT"); break;       //!< Reset due to task watchdog
        case 9 : Serial.println ("ESP_RST_WDT"); break;            //!< Reset due to other watchdogs
        case 10 : Serial.println ("ESP_RST_DEEPSLEEP"); break;     //!< Reset after exiting deep sleep mode
        case 11 : Serial.println ("ESP_RST_BROWNOUT"); break;      //!< Brownout reloadSettings (software or hardware)
        case 13 : Serial.println ("ESP_RST_SDIO"); break;          //!< Reset over SDIO
    }
}

static void onChange(GxFT5436::Change change, void* param) {

//    Log.log(change.toString());

    View view = Screen.getView();

    switch (view) {
        case MENU: {
            Screen.menu->scroll(change.diffY);
        }
    }

}

void setup(void) {
    Serial.begin(115200);


//    Log.logf("Free heap: %d\n", ESP.getFreeHeap());

    if (!SPIFFS.begin()) {
        Log.log("SPIFFS initialisation failed!");
    }

    Updater.init();

    Settings.init();
    S_STATUS settingsStatus = Settings.load();
    SettingsClass::DataSource selected[3];
    Settings.loadSelected(selected);

    Screen.init(selected);

    Log.logf("Firmware version: %s(%d)\n", Updater.firmware.toString().c_str(), Updater.firmware.toInt());
    Log.logf("Filesystem version: %s(%d), target: %s(%d)\n", Updater.filesystemCurrent.toString().c_str(), Updater.filesystemCurrent.toInt(),  Updater.filesystemTarget.toString().c_str(), Updater.filesystemTarget.toInt());
    if(Updater.filesystemTarget != Updater.filesystemCurrent) {
        Screen.setBrightness(255);
        proceed = false;
        Log.log("Filesystem version does not match target version. Trying to update");

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
                    Log.log("HTTP_UPDATE_NO_UPDATES");
                    break;

                case HTTP_UPDATE_OK:
                    Log.log("HTTP_UPDATE_OK");
                    Settings.save();
                    Screen.appendToPrompt("\nUpdate successful\n Restarting in 3 seconds");
                    Screen.tick();
                    delay(3000);
                    esp_restart();
            }
        });
    } else  {

         Networking.connectWiFi((char *)Settings.general[WIFI_SSID]->getString().c_str(), (char *)Settings.general[WIFI_PASS]->getString().c_str());

         Data.POST();
         Data.init();

    //        Data.touch.addOnEvent(onEvent);
        Data.touch.addOnChange(onChange);

    //  TwoWire twoWire(1);
    //  twoWire.setPins(21, 22);
    //
    //  if (!mcp23008.begin_I2C(0x20, &twoWire)) {
    //      //if (!mcp.begin_SPI(CS_PIN)) {
    //      Log.log("Error.");
    //      while (1);
    //  }

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
    }
  Log.log("setup() complete");
}

unsigned  long t15;
void loop() {

    Updater.loop();

    t15 = millis();


    Log.logf("total: %d, block: %d\n", ESP.getFreeHeap(), ESP.getMaxAllocHeap());

    Screen.tick();

    std::stringstream ss;
    ss << millis()-t15;
    Networking.sendEvent("frametime", ss.str());
}