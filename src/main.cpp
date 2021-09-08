#include <Wire.h>
#include <SPI.h>

#include "Networking.h"
#include "Data.h"
#include "Settings.h"
#include "Screen.h"

#include <WiFi.h>
#include "time.h"


#include "mcp2515.h"
#include "RTClib.h"
#include "TFT_eSPI.h"
#include "GxFT5436.h"
#include <Adafruit_MCP23X08.h>
#include "ADS1X15.h"


const int ledPin = 32;

// // setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
int brightness = 255;


TFT_eSPI tft = TFT_eSPI();

void loadFonts() {
  if (!SPIFFS.begin()) {
      Serial.println("SPIFFS initialisation failed!");
      while (1) yield(); // Stay here twiddling thumbs waiting
    }
    Serial.println("\r\nSPIFFS available!");

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
      Serial.println("\r\nFont missing in SPIFFS, did you upload it?");
      while(1) yield();
    }
    else Serial.println("\r\nFonts found OK.");

}


int x = 0, fps = 0;

unsigned long total, t;



Networking networking;
//Adafruit_MCP23X08 mcp23008;
Data data;


void setup(void) {
  Serial.begin(115200);
  Serial.println("Hello");
  tft.init();
  tft.setRotation(3);
  tft.invertDisplay(1);


  tft.fillScreen(TFT_BLUE);

//  TwoWire twoWire(1);
//  twoWire.setPins(21, 22);
//
//  if (!mcp23008.begin_I2C(0x20, &twoWire)) {
//      //if (!mcp.begin_SPI(CS_PIN)) {
//      Serial.println("Error.");
//      while (1);
//  }


  loadFonts();


    data.init();

  Settings::getInstance()->loadDefault();
  Settings::getInstance()->load();

  Screen::getInstance()->init(&tft, &data);
  Screen::getInstance()->reset();

//  Screen::getInstance()->updateNeedle(0, cos(90/PI/18)/2+0.5);
//  Screen::getInstance()->updateNeedle(1, cos(52/PI/18)/2+0.5);


  networking.connectWiFi();
  networking.serverSetup();

  //configure LED PWM functionalitites
  ledcSetup(ledChannel, freq, resolution);

  //attach the channel to the GPIO to be controlled
  ledcAttachPin(ledPin, ledChannel);
  ledcWrite(0, 255);
}



void loop() {

//    Serial.print(pcTaskGetTaskName(NULL));
//    Serial.print(" started on core ");
//    Serial.println(xPortGetCoreID());

    total = millis();
    t = millis();

    if(Screen::getInstance()->shallWeReset) {
        Screen::getInstance()->reset();
        Screen::getInstance()->updateText(true, fps);
    }

//
//    // Serial.print("Touch time: ");
//    // Serial.println(millis()-t);
//    t = millis();
//
//    // Serial.print("Server time: ");
//    // Serial.println(millis()-t);
//    t = millis();
//
    Screen::getInstance()->updateText(false, fps);
//     Serial.print("Text update time: ");
//     Serial.println(millis()-t);
// rpmVal = rpmRead();

//    float oilTemp = (oilBeta*roomOilTemp/(oilBeta+(roomOilTemp*math.log(oilRes/s/roomOilResist)))) - 273.15
//    float oilTRes = 10110.0*data.data.adc[1]/(3.3-data.data.adc[1]);
//    float oilPRes = 101.9*data.data.adc[0]/(3.3-data.data.adc[0]);
//    float oilTemp = 3800.0*(21.8 + 273.15)/(3800.0+((21.8 + 273.15)*log(oilTRes/58000.0))) - 273.15;
//    float oilPress = (oilPRes-3.0)/(160.0-3.0)*10.0;


//    Serial.print("resistance 1: ");
//    Serial.print(oilTRes);
//    Serial.print(", resistance 2: ");
//    Serial.print(oilPRes);
//    Serial.print(", Temp: ");
//    Serial.print(oilTemp);
//    Serial.print(", press: ");
//    Serial.println(oilPress);

    Screen::getInstance()->updateNeedle(0, sin(x/PI/18)/2+0.5);
//Screen::getInstance()->updateNeedle(0, oilTemp);
//    test();
    // Screen::getInstance()->updateNeedle(0, rpmVal/8000.0);
//    // rpmVal = rpmRead();
    Screen::getInstance()->updateNeedle(1, cos(x/PI/18)/2+0.5);
//Screen::getInstance()->updateNeedle(1, oilPress);
    x+=2;
    if(x>=360) {
      x-=360;
    }

//

//    Serial.print("Frametime: ");
////    fps = millis()-t;
////    delay(10);
//    Serial.print(millis()-t);
//    Serial.print(", loop time: ");
//    Serial.println(millis()-total);
}