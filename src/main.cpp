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
GxFT5436 touch = GxFT5436(/*SDA=*/21, /*SCL=*/22,/*RST=*/-1);

void f(t_httpUpdate_return status) {
    switch (status) {
        case HTTP_UPDATE_FAILED:
            Log.logf("HTTP_UPDATE_FAILD Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            Screen::getInstance()->appendToPrompt("Update failed " + (String) httpUpdate.getLastError() + ": " +
                                                  httpUpdate.getLastErrorString().c_str() + "\nReboot to try again", 4, true);
            break;

        case HTTP_UPDATE_NO_UPDATES:
            Log.log("HTTP_UPDATE_NO_UPDATES");
            break;

        case HTTP_UPDATE_OK:
            Log.log("HTTP_UPDATE_OK");
            Settings::getInstance()->save();
            Screen::getInstance()->appendToPrompt("Update successful\n Restarting in 3 seconds", 4, true);
            delay(3000);
            esp_restart();
    }


}

static void action(GxFT5436::Event event) {

    if(event.type == GxFT5436::SINGLE_CLICK) {
        
        int16_t x = event.startX;
        int16_t y = event.startY;

        Log.logf("Single touch at %d,%d\n", x, y);

        View view = Screen::getInstance()->getView();

        switch (view) {
            case PROMPT: {
                //dismiss menu
                Screen::getInstance()->setGaugeMode();
                break;
            }
            case GAUGES: {
                //show menu
                if(Screen::getInstance()->getView() != PROMPT && x > Settings::getInstance()->general[WIDTH]->get<int>()/2 - Settings::getInstance()->general[NEEDLE_CENTER_OFFSET]->get<int>() && x < Settings::getInstance()->general[WIDTH]->get<int>()/2 + Settings::getInstance()->general[NEEDLE_CENTER_OFFSET]->get<int>() && y < Settings::getInstance()->general[HEIGHT]->get<int>()/2) {
                    Screen::getInstance()->showPrompt("SSID: " + String((char *)Settings::getInstance()->general[SSID]->getString().c_str()) + "\npass: " + String((char *)Settings::getInstance()->general[PASS]->getString().c_str()) + "\nIP: " + WiFi.localIP().toString() + "\nFW: " + getCurrentFirmwareVersionString());
                }

                //change gauge

                Side side = SIDE_LAST;
                if(x < Settings::getInstance()->general[WIDTH]->get<int>()/2-Settings::getInstance()->general[NEEDLE_CENTER_OFFSET]->get<int>())
                    side = LEFT;
                else if(x > Settings::getInstance()->general[WIDTH]->get<int>()/2+Settings::getInstance()->general[NEEDLE_CENTER_OFFSET]->get<int>())
                    side = RIGHT;
                else if(x > Settings::getInstance()->general[WIDTH]->get<int>()/2-Settings::getInstance()->general[NEEDLE_CENTER_OFFSET]->get<int>() &&
                        x < Settings::getInstance()->general[WIDTH]->get<int>()/2+Settings::getInstance()->general[NEEDLE_CENTER_OFFSET]->get<int>() &&
                        y > Settings::getInstance()->general[HEIGHT]->get<int>()/2)
                    side = MID;
                
                if(side != SIDE_LAST) {
                    Settings::DataSource selected[3];
                    Screen::getInstance()->getSelected(selected);

                    Log.logf("Current data: %s\n", Settings::getInstance()->dataSourceString[selected[side]].c_str());
                    do {
                        selected[side] = static_cast<Settings::DataSource>(selected[side]+1);
                        if(selected[side] == Settings::LAST)
                            selected[side] = static_cast<Settings::DataSource>(0);
                    } while (!Settings::getInstance()->dataDisplay[selected[side]].enable);
                    Log.logf("Changing to: %s\n", Settings::getInstance()->dataSourceString[selected[side]].c_str());
                    Screen::getInstance()->setSelected(side, selected[side]);
                    Settings::getInstance()->saveSelected(selected);
                }

            }
        }






    }
//    //Slide right
//    else if((abs(startX[i]-endX[i]) > SLIDE_ALONG_DISTANCE) && (abs(startY[i]-endY[i]) < SLIDE_ACROSS_DISTANCE) && (endX[i] > startX[i])) {
//        Log.logf("Slide right from %d,%d", startX[i], endY[i]);
//    }
//    //Slide left
//    else if((abs(startX[i]-endX[i]) > SLIDE_ALONG_DISTANCE) && (abs(startY[i]-endY[i]) < SLIDE_ACROSS_DISTANCE) && (endX[i] < startX[i])) {
//        Log.logf("Slide left from %d,%d", startX[i], endY[i]);
//    }
//    //Slide down
//    else if((abs(startX[i]-endX[i]) < SLIDE_ACROSS_DISTANCE) && (abs(startY[i]-endY[i]) > SLIDE_ALONG_DISTANCE) && (endY[i] > startY[i])) {
//        Log.logf("Slide down from %d,%d", startX[i], endY[i]);
//    }
//    //Slide up
//    else if((abs(startX[i]-endX[i]) < SLIDE_ACROSS_DISTANCE) && (abs(startY[i]-endY[i]) > SLIDE_ALONG_DISTANCE) && (endY[i] < startY[i])) {
//        Log.logf("Slide up from %d,%d", startX[i], endY[i]);
//    }

}


void setup(void) {
  Serial.begin(115200);

  if (!SPIFFS.begin()) {
      Log.log("SPIFFS initialisation failed!");
  }
  Log.log("SPIFFS available");

  tft.init();
  tft.setRotation(3);
  tft.invertDisplay(1);


  Settings::getInstance()->init();

  Settings::getInstance()->loadDefault();
  Settings::getInstance()->load();
  Settings::DataSource selected[SIDE_LAST];
  Settings::getInstance()->loadSelected(selected);

  Screen::getInstance()->init(&tft, &data);
  Screen::getInstance()->setSelected(selected);

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

      Screen::getInstance()->showPrompt("Filesystem version does not match target version\ncurrent: " +
      getCurrentFilesystemVersionString() +
      ", target: " +
      getTargetFilesystemVersionString() +
      "\nCreate an AP with following credentials:\nSSID: \"" +
      (char *)Settings::getInstance()->general[SSID]->getString().c_str() +
      "\", pass: \"" +
      (char *)Settings::getInstance()->general[PASS]->getString().c_str() +
      "\"",
      4, true);

      networking.connectWiFi(TIME_INFINITY, (char *)Settings::getInstance()->general[SSID]->getString().c_str(), (char *)Settings::getInstance()->general[PASS]->getString().c_str());
      while(WiFi.status() != WL_CONNECTED){
          delay(50);
      }
      Screen::getInstance()->appendToPrompt("WiFi connected, updating... this may take a while", 4, true);
      updater.updateFS(getTargetFilesystemVersionString(), f);
  }

  if(proceed) {

      networking.connectWiFi(CONNECTING_TIME, (char *)Settings::getInstance()->general[SSID]->getString().c_str(), (char *)Settings::getInstance()->general[PASS]->getString().c_str());

      data.init();

      while(data.data.i2cBusy)
          delay(1);
      data.data.i2cBusy = true;

      if(!touch.init(&Serial))
          Log.log("GxFT5436 not found");
      else {
          Log.log("GxFT5436 found");
          touch.onEvent(action);
          touch.enableInterrupt(33, &(data.data.i2cBusy), 1, 0);
      }

      data.data.i2cBusy = false;

    //  tft.fillScreen(TFT_BLUE);

    //  TwoWire twoWire(1);
    //  twoWire.setPins(21, 22);
    //
    //  if (!mcp23008.begin_I2C(0x20, &twoWire)) {
    //      //if (!mcp.begin_SPI(CS_PIN)) {
    //      Log.log("Error.");
    //      while (1);
    //  }



      Screen::getInstance()->reset();
      Screen::getInstance()->setGaugeMode();

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

        std::stringstream ss;
        ss << millis()-t15;
        networking.sendEvent("frametime", ss.str());
    } else
        delay(1);
}