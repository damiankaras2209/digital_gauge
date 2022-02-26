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

//PWM
const int ledPin = 32;
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
int brightness = 255;

bool proceed = true;
bool updateChecked = false;

UpdaterClass updater;

void print_reset_reason(esp_reset_reason_t reason)
{
    switch(reason)
    {
        case 1 : Serial.println ("ESP_RST_UNKNOWN"); break;        //!< Reset reason can not be determined
        case 3 : Serial.println ("ESP_RST_POWERON"); break;        //!< Reset due to power-on event
        case 4 : Serial.println ("ESP_RST_EXT"); break;            //!< Reset by external pin (not applicable for ESP32)
        case 5 : Serial.println ("ESP_RST_SW"); break;             //!< Software reset via esp_restart
        case 6 : Serial.println ("ESP_RST_PANIC"); break;          //!< Software reset due to exception/panic
        case 7 : Serial.println ("ESP_RST_INT_WDT"); break;        //!< Reset (software or hardware) due to interrupt watchdog
        case 8 : Serial.println ("ESP_RST_TASK_WDT"); break;       //!< Reset due to task watchdog
        case 9 : Serial.println ("ESP_RST_WDT"); break;            //!< Reset due to other watchdogs
        case 10 : Serial.println ("ESP_RST_DEEPSLEEP"); break;     //!< Reset after exiting deep sleep mode
        case 11 : Serial.println ("ESP_RST_BROWNOUT"); break;      //!< Brownout reset (software or hardware)
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

    Log.log(ESP.getFreeHeap());

    if (!SPIFFS.begin()) {
        Log.log("SPIFFS initialisation failed!");
    }

    Serial.println("ESP32 reset reason: ");
    print_reset_reason(esp_reset_reason());

//    if(esp_reset_reason() != ESP_RST_POWERON && esp_reset_reason() != ESP_RST_UNKNOWN && esp_reset_reason() != ESP_RST_SW)
//        Settings.clear();

    Settings.init();
    Settings.loadDefault();
    Settings.load();
    SettingsClass::DataSource selected[SIDE_LAST];
    Settings.loadSelected(selected);

    Screen.init();
    Screen.gauges->setSelected(selected);

    ledcSetup(ledChannel, freq, resolution);
    ledcAttachPin(ledPin, ledChannel);
    ledcWrite(0, 0);

    Log.logf("Firmware version: %s\n", getCurrentFirmwareVersionString().c_str());
    Log.logf("Filesystem current version: %s\n", getCurrentFilesystemVersionString().c_str());
    Log.logf("Filesystem target version: %s\n", getTargetFilesystemVersionString().c_str());
    if(getCurrentFilesystemVersion() != getTargetFilesystemVersion()) {
        ledcWrite(0, 255);
        proceed = false;
        Log.log("Filesystem version does not match target version. Trying to update");

        Screen.showPrompt("Filesystem version does not match target version\ncurrent: " +
                          getCurrentFilesystemVersionString() +
                          ", target: " +
                          getTargetFilesystemVersionString() +
                          "\nCreate an AP with following credentials:\nSSID: \"" +
                          (char *)Settings.general[WIFI_SSID]->getString().c_str() +
                          "\", pass: \"" +
                          (char *)Settings.general[WIFI_PASS]->getString().c_str() +
                          "\"",
        4, true);

        Networking.connectWiFi(TIME_INFINITY, (char *)Settings.general[WIFI_SSID]->getString().c_str(), (char *)Settings.general[WIFI_PASS]->getString().c_str());
        while(WiFi.status() != WL_CONNECTED){
            delay(50);
        }
        Screen.appendToPrompt("\nWiFi connected, updating... this may take a while");
        updater.updateFS(getTargetFilesystemVersionString(), [](t_httpUpdate_return status) {
            switch (status) {
                case HTTP_UPDATE_FAILED:
                    Log.logf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
                    Screen.appendToPrompt("\nUpdate failed " + (String) httpUpdate.getLastError() + ": " +
                        httpUpdate.getLastErrorString().c_str() + "\nReboot to try again");
                    break;
                case HTTP_UPDATE_NO_UPDATES:
                    Log.log("HTTP_UPDATE_NO_UPDATES");
                    break;

                case HTTP_UPDATE_OK:
                    Log.log("HTTP_UPDATE_OK");
                    Settings.save();
                    Screen.appendToPrompt("\nUpdate successful\n Restarting in 3 seconds");
                    delay(3000);
                    esp_restart();
            }
        });
    }

    if(proceed) {


        WiFi.onEvent([](WiFiEvent_t event, WiFiEventInfo_t info) {
            Log.logf("Connected to %s,IP: %s\n ", WiFi.SSID().c_str(),IPAddress(info.ap_staipassigned.ip.addr).toString().c_str());

            if (!MDNS.begin(HOSTNAME)) { //http://esp32.local
                Log.log("Error setting up MDNS responder!");
            }

            Log.logf("mDNS responder started, hostname: http://%s.local\n", HOSTNAME);

            Data.adjustTime(&Data.data);
            Networking.serverSetup();
            }, SYSTEM_EVENT_STA_GOT_IP);

         Networking.connectWiFi(CONNECTING_TIME, (char *)Settings.general[WIFI_SSID]->getString().c_str(), (char *)Settings.general[WIFI_PASS]->getString().c_str());

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



        Screen.reset();
        Screen.switchView(GAUGES);

        ledcWrite(0, 255);
  }
  Log.log("setup() complete");
}

unsigned  long t15;
void loop() {

    if(!updateChecked && WiFi.status() == WL_CONNECTED) {
//        updater.checkForUpdate();
        updateChecked = true;
    }


    Screen.tick();

    if(proceed) {
        t15 = millis();
//        delay(1);

        std::stringstream ss;
        ss << millis()-t15;
        Networking.sendEvent("frametime", ss.str());
    } else
        delay(1);
}