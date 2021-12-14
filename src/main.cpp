#include <Wire.h>
#include <SPI.h>

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

TFT_eSPI tft = TFT_eSPI();
Networking networking;
UpdaterClass updater;
Data data;

Settings::DataSource selected[SIDE_LAST];

volatile ulong touchDetectedTime = 0;

void IRAM_ATTR touchStart() {
    touchDetectedTime = millis();
}

#define SINGLE_POINT_DISTANCE 10
#define SLIDE_ALONG_DISTANCE 40
#define SLIDE_ACROSS_DISTANCE 15

[[noreturn]] void touch(void * pvParameters) {
    Log.logf("%s started on core %d", pcTaskGetTaskName(NULL), xPortGetCoreID());
//    Log.log(" started on core ");
//    Log.log(xPortGetCoreID());


    Data::DataStruct *params = (Data::DataStruct*)pvParameters;
    Settings *settings = Settings::getInstance();

    unsigned long last[5];
    bool down[5] = {false, false, false, false, false};
    uint16_t startX[5], startY[5];
    uint16_t endX[5], endY[5];


    while(1) {
        //        Log.log("millis() - touchDetectedTime: ");
        //        Log.log (millis() - touchDetectedTime);
        if(millis() - touchDetectedTime < 10) {
            while(params->i2cBusy)
                delay(1);
            params->i2cBusy = true;


            GxFT5436* touch = params->touchPtr;
            params->touchInfo = touch->scanMultipleTouch();

            bool detected[5] = {false, false, false, false, false};

            for (uint8_t i = 0; i < params->touchInfo.touch_count; i++) {

                uint16_t x1 = params->touchInfo.x[i];
                params->touchInfo.x[i] = params->touchInfo.y[i];
                params->touchInfo.y[i] = 319-x1;


                uint8_t id = params->touchInfo.id[i];
                if(!down[id]) {
                    down[id] = true;
                    Log.logf("Touch down (%d)", id);
                    startX[id] = params->touchInfo.x[i];
                    startY[id] = params->touchInfo.y[i];
                }
                detected[id] = true;
                endX[id] = params->touchInfo.x[i];
                endY[id] = params->touchInfo.y[i];

                Log.logf("touch id: %d, last: %lu (%d,%d)", params->touchInfo.id[i], millis() - last[id], params->touchInfo.x[i], params->touchInfo.y[i]);
            }

            for (uint8_t i = 0; i < 5; i++) {
                if(!detected[i] && down[i]) {
                    down[i] = false;
                    Log.logf("Touch up (%hu) start(%hu, %hu) end(%hu,%hu) ", i, startX[i], startY[i], endX[i], endY[i]);
                    //Single touch
                    if((abs(startX[i]-endX[i]) < SINGLE_POINT_DISTANCE) && (abs(startY[i]-endY[i]) < SINGLE_POINT_DISTANCE)) {

                        Log.logf("Single touch at %d,%d", endX[i], endY[i]);

                        //show menu
                        if(Screen::getInstance()->getView() != PROMPT && endX[i] > settings->visual.width/2 - settings->visual.needleCenterOffset && endX[i] < settings->visual.width/2 + settings->visual.needleCenterOffset && endY[i] < settings->visual.height/2) {
                            Screen::getInstance()->showPrompt("SSID: " + String((char *)Settings::getInstance()->general.ssid) + "\npass: " + String((char *)Settings::getInstance()->general.pass) + "\nIP: " + WiFi.localIP().toString() + "\nFW: " + getCurrentFirmwareVersionString());
                        }

                        //dismiss menu
                        else if(Screen::getInstance()->getView() == PROMPT)
                            Screen::getInstance()->setGaugeMode();


                        //change left gauge
                        else if(Screen::getInstance()->getView() == GAUGES && endX[i] < settings->visual.width/2-settings->visual.needleCenterOffset) {
                            Log.logf("Current data: %s", settings->dataSourceString[selected[LEFT]].c_str());
                            do {
                                selected[LEFT] = static_cast<Settings::DataSource>(selected[LEFT]+1);
                                if(selected[LEFT] == Settings::LAST)
                                    selected[LEFT] = static_cast<Settings::DataSource>(0);
                            } while (!settings->dataDisplay[selected[LEFT]].enable);
                            Log.logf("Changing to: %s\n", settings->dataSourceString[selected[LEFT]].c_str());
                            settings->saveSelected(selected);
                        }

                        //change right gauge
                        else if(Screen::getInstance()->getView() == GAUGES && endX[i] > settings->visual.width/2+settings->visual.needleCenterOffset) {
                            Log.logf("Current data: %s", settings->dataSourceString[selected[RIGHT]].c_str());
                            do {
                                selected[RIGHT] = static_cast<Settings::DataSource>(selected[RIGHT]+1);
                                if(selected[RIGHT] == Settings::LAST)
                                    selected[RIGHT] = static_cast<Settings::DataSource>(0);
                            } while (!settings->dataDisplay[selected[RIGHT]].enable);
                            Log.logf("Changing to: %s\n", settings->dataSourceString[selected[RIGHT]].c_str());
                            settings->saveSelected(selected);
                        }

                    }
                    //Slide right
                    else if((abs(startX[i]-endX[i]) > SLIDE_ALONG_DISTANCE) && (abs(startY[i]-endY[i]) < SLIDE_ACROSS_DISTANCE) && (endX[i] > startX[i])) {
                        Log.logf("Slide right from %d,%d", startX[i], endY[i]);
                    }
                    //Slide left
                    else if((abs(startX[i]-endX[i]) > SLIDE_ALONG_DISTANCE) && (abs(startY[i]-endY[i]) < SLIDE_ACROSS_DISTANCE) && (endX[i] < startX[i])) {
                        Log.logf("Slide left from %d,%d", startX[i], endY[i]);
                    }
                    //Slide down
                    else if((abs(startX[i]-endX[i]) < SLIDE_ACROSS_DISTANCE) && (abs(startY[i]-endY[i]) > SLIDE_ALONG_DISTANCE) && (endY[i] > startY[i])) {
                        Log.logf("Slide down from %d,%d", startX[i], endY[i]);
                    }
                    //Slide up
                    else if((abs(startX[i]-endX[i]) < SLIDE_ACROSS_DISTANCE) && (abs(startY[i]-endY[i]) > SLIDE_ALONG_DISTANCE) && (endY[i] < startY[i])) {
                        Log.logf("Slide up from %d,%d", startX[i], endY[i]);
                    }
                }
            }

            params->i2cBusy = false;
        }
        delay(7);
    }
}






void loadFonts() {
  if (!SPIFFS.begin()) {
      Log.log("SPIFFS initialisation failed!");
      while (1) yield(); // Stay here twiddling thumbs waiting
    }
    Log.log("SPIFFS available!");

    // ESP32 will crash if any of the fonts are missing
    bool font_missing = false;
    if (!SPIFFS.exists("/GaugeHeavy10.vlw")) font_missing = true;
    if (!SPIFFS.exists("/GaugeHeavy12.vlw")) font_missing = true;
    if (!SPIFFS.exists("/GaugeHeavy16.vlw")) font_missing = true;
//    if (!SPIFFS.exists("/GaugeHeavy20.vlw")) font_missing = true;
//    if (!SPIFFS.exists("/GaugeHeavy26.vlw")) font_missing = true;
    if (!SPIFFS.exists("/GaugeHeavy36.vlw")) font_missing = true;
    if (!SPIFFS.exists("/GaugeHeavyNumbers12.vlw")) font_missing = true;

    if (font_missing)
    {
      Log.log("Font missing in SPIFFS, did you upload it?");
      while(1) yield();
    }
    else Log.log("Fonts found OK.");

}

void f(t_httpUpdate_return status) {
    switch (status) {
        case HTTP_UPDATE_FAILED:
            Log.logf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            Screen::getInstance()->appendToPrompt("Update failed " + (String) httpUpdate.getLastError() + ": " +
                                                  httpUpdate.getLastErrorString().c_str() + "\nReboot to try again");
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Log.log("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            Log.log("HTTP_UPDATE_OK");
            Settings::getInstance()->save();
            Screen::getInstance()->appendToPrompt("Update successful\n Restarting in 3 seconds");
            delay(3000);
            esp_restart();
    }


}


void setup(void) {
  Serial.begin(115200);


  selected[LEFT] = Settings::ADS1115_1;
  selected[RIGHT] = Settings::ADS1115_0;
  selected[MID]= Settings::VOLTAGE;

  tft.init();
  tft.setRotation(3);
  tft.invertDisplay(1);


  loadFonts();

  Settings::getInstance()->loadDefault();
  Settings::getInstance()->load();
  Settings::getInstance()->loadSelected(selected);

  Screen::getInstance()->init(&tft, &data, selected);

  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);
  ledcWrite(0, 0);

  Log.logf("Firmware version: %s", getCurrentFirmwareVersionString().c_str());
  Log.logf("Filesystem current version: %s", getCurrentFilesystemVersionString().c_str());
  Log.logf("Filesystem target version: %s", getTargetFilesystemVersionString().c_str());
  if(getCurrentFilesystemVersion() != getTargetFilesystemVersion()) {
      ledcWrite(0, 255);
      proceed = false;
      Log.log("Filesystem version does not match target version. Trying to update");

      Screen::getInstance()->showPrompt("Filesystem version does not match target version\nCurrent: " +
      getCurrentFilesystemVersionString() +
      "\tTarget: " +
      getTargetFilesystemVersionString() +
      "\nCreate an AP with following credentials:\nSSID: \"" +
      (char *)Settings::getInstance()->general.ssid +
      "\", pass: \"" +
      (char *)Settings::getInstance()->general.pass +
      "\""
      );

      networking.connectWiFi(TIME_INFINITY, (char *)Settings::getInstance()->general.ssid, (char *)Settings::getInstance()->general.pass);
      while(WiFi.status() != WL_CONNECTED){
          delay(50);
      }
      Screen::getInstance()->appendToPrompt("WiFi connected, updating... this may take a few minutes");
      updater.updateFS(getTargetFilesystemVersionString(), f);
  }

  if(proceed) {

      networking.connectWiFi(CONNECTING_TIME, (char *)Settings::getInstance()->general.ssid, (char *)Settings::getInstance()->general.pass);

      data.init();

      if(data.data.GxFT5436Available) {
          pinMode(33, INPUT_PULLDOWN);
          attachInterrupt(digitalPinToInterrupt(33), touchStart, RISING);

          TaskHandle_t touchHandle;
          if(!xTaskCreatePinnedToCore(touch,
              "touch",
              4*1024,
              &(data.data),
              1,
              &touchHandle, 0))
                    Log.log("Failed to start touch task");
      }
    //  tft.fillScreen(TFT_BLUE);

    //  TwoWire twoWire(1);
    //  twoWire.setPins(21, 22);
    //
    //  if (!mcp23008.begin_I2C(0x20, &twoWire)) {
    //      //if (!mcp.begin_SPI(CS_PIN)) {
    //      Log.log("Error.");
    //      while (1);
    //  }


    //  loadFonts();




      Screen::getInstance()->reset();
      Screen::getInstance()->setGaugeMode();

    //  networking.connectWiFi(false);
    //  networking.serverSetup();

      ledcWrite(0, 255);
  }
  Log.log("setup() complete");
}

unsigned  long t15;
void loop() {

    if(!updateChecked && WiFi.status() == WL_CONNECTED) {
        updater.checkForUpdate();
        updateChecked = true;
    }

    if(proceed) {
        t15 = millis();
        Screen::getInstance()->tick();
//        delay(1);
#ifdef LOG_FRAMETIME
        Log.logf("Frametime: %lu", millis()-t15);
#endif
        std::stringstream ss;
        ss << millis()-t15;
        networking.sendEvent("frametime", ss.str());
    } else
        delay(1);
}