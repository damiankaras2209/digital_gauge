// Display Library for parallel interface e-paper panels from Dalian Good Display Co., Ltd.: www.e-paper-display.com
//
// based on Demo Examples from Good Display, available here: http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
//
// Class GxFT5436 : FT5436 touch driver class for GDE060F3-T on DESTM32-T parallel interface e-paper display from Dalian Good Display Inc.
//
// Author : J-M Zingg
//
// Version : see library.properties
//
// License: GNU GENERAL PUBLIC LICENSE V3, see LICENSE
//
// Library: https://github.com/ZinggJM/GxEPD_HD

#include "GxFT5436.h"

#include <utility>

#define DiagOut if(_pDiagnosticOutput) (*_pDiagnosticOutput)

GxFT5436::GxFT5436(int8_t sda, int8_t scl, int8_t rst) : I2C(0), _sda(sda), _scl(scl), _rst(rst)
{
    _loopData.touch = this;
  _prev_idx = 0;
  _act_idx = 1;
  _info[0].clear();
  _info[1].clear();
}

bool GxFT5436::init(Stream* pDiagnosticOutput)
{
  _pDiagnosticOutput = pDiagnosticOutput;
  _loopData.diagOut = _pDiagnosticOutput;
  _info[0].clear();
  _info[1].clear();
  if (_rst >= 0)
  {
    pinMode(_rst, OUTPUT);
    digitalWrite(_rst, LOW);
    delay(20);
    digitalWrite(_rst, HIGH);
    delay(100);
  }
  I2C.begin(_sda, _scl);
  I2C.beginTransmission(FT5436_I2C_ADDR);
  uint8_t error = I2C.endTransmission();
  if (error != 0)
  {
    DiagOut.println("GxFT5436::init() - I2C failed for address 0x"); DiagOut.print(FT5436_I2C_ADDR, HEX);
    return false;
  }
  DiagOut.println("GxFT5436 init() successful");
  I2C_Write(FT5436_I2C_ADDR, FT_REG_DEV_MODE, 0);
//  I2C_Write(FT5436_I2C_ADDR, FT_REG_THGROUP, 1);
//  I2C_Write(FT5436_I2C_ADDR, 0x81, 1);
//  I2C_Write(FT5436_I2C_ADDR, 0x82, 1);
  I2C_Write(FT5436_I2C_ADDR, FT_REG_POINT_RATE, 14);
//  DiagOut.print("THGROUP: ");
//  DiagOut.println(I2C_Read(FT5436_I2C_ADDR, FT_REG_THGROUP));
//  DiagOut.print("THPEAK: ");
//  DiagOut.println(I2C_Read(FT5436_I2C_ADDR, 0x81));
//  DiagOut.print("ERROR: ");
//  DiagOut.println(I2C_Read(FT5436_I2C_ADDR, 0xA9), HEX);
    return true;
}

GxFT5436::TouchInfo GxFT5436::scan()
{
  uint32_t start = micros();
  I2C_Read(FT5436_I2C_ADDR, FT_REG_DEV_MODE, _registers, POINT_READ_BUF);
  uint32_t elapsed1 = micros() - start;
  uint8_t touch_count = _registers[FT_TD_STATUS] & FT_MAX_ID;
  if (touch_count > CFG_MAX_TOUCH_POINTS)
  {
//    DiagOut.print("scan() got invalid touch_count: "); DiagOut.print(touch_count); DiagOut.print(" 0x"); DiagOut.println(_registers[FT_TD_STATUS], HEX);
    I2C_Read(FT5436_I2C_ADDR, FT_REG_DEV_MODE, _registers, POINT_READ_BUF);
    touch_count = _registers[FT_TD_STATUS] & FT_MAX_ID;
    if (touch_count > CFG_MAX_TOUCH_POINTS) touch_count = 0;
  }
  //std::swap(_act_idx, _prev_idx);
  int16_t t = _act_idx; _act_idx = _prev_idx; _prev_idx = t;
  _info[_act_idx].touch_count = touch_count;
  for (uint8_t i = 0; i < CFG_MAX_TOUCH_POINTS; i++)
  {
    _info[_act_idx].x[i] = 0;
    _info[_act_idx].y[i] = 0;
  }

  for(uint8_t i=0; i<touch_count; i++)
  {
      _info[_act_idx].event[i] = (_registers[3 + i * 6] & 0xc0) >> 6;
      _info[_act_idx].id[i] = (_registers[5 + i * 6] & 0xf0) >> 4;
      _info[_act_idx].x[i] = ((_registers[3 + i * 6] & 0x0f) << 8) | _registers[4 + i * 6];
      _info[_act_idx].y[i] = ((_registers[5 + i * 6] & 0x0f) << 8) | _registers[6 + i * 6];
  }

  uint32_t elapsed2 = micros() - start;
  if (touch_count > 0)
  {
    //DiagOut.print("scan() "); DiagOut.print(elapsed1); DiagOut.print(" "); DiagOut.println(elapsed2);
  }
  (void) elapsed1;
  (void) elapsed2;
  return _info[_act_idx];
}

volatile ulong touchDetectedTime = 0;

void GxFT5436::touchStart() {
    touchDetectedTime = millis();
}

bool GxFT5436::enableInterrupt(int8_t interrupt, volatile bool* i2cBusy, int8_t priority, int8_t core) {
    _loopData.i2cBusy = i2cBusy;
    pinMode(interrupt, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(interrupt), touchStart, RISING);

    TaskHandle_t touchHandle;
    return xTaskCreatePinnedToCore(touch,
                                "touch",
                                4*1024,
                                &(_loopData),
                                priority,
                                &touchHandle, core);
}

void GxFT5436::onEvent(GxFT5436::Callback callback) {
    _loopData.action = callback;
}

#define SINGLE_POINT_DISTANCE 10
#define SLIDE_ALONG_DISTANCE 40
#define SLIDE_ACROSS_DISTANCE 15



[[noreturn]] void GxFT5436::touch(void * pvParameters) {
    auto data = (LoopData*)pvParameters;

    data->diagOut->printf("%s started on core %d", pcTaskGetTaskName(NULL), xPortGetCoreID());

    unsigned long last[5];
    bool down[5] = {false, false, false, false, false};
    uint16_t startX[5], startY[5];
    uint16_t endX[5], endY[5];


    while(1) {
//        data->diagOut->print("millis() - touchDetectedTime: ");
//        data->diagOut->print (millis() - touchDetectedTime);
        if(millis() - touchDetectedTime < 10) {
            while(*(data->i2cBusy)) {
                delay(1);
            }
            *(data->i2cBusy) = true;


            GxFT5436::TouchInfo touchInfo = data->touch->scan();

            bool detected[5] = {false, false, false, false, false};

            for (uint8_t i = 0; i < touchInfo.touch_count; i++) {

                uint16_t x1 = touchInfo.x[i];
                touchInfo.x[i] = touchInfo.y[i];
                touchInfo.y[i] = 319-x1;


                uint8_t id = touchInfo.id[i];
                if(!down[id]) {
                    down[id] = true;
                    data->diagOut->printf("Touch down (%d)", id);
                    startX[id] = touchInfo.x[i];
                    startY[id] = touchInfo.y[i];
                }
                detected[id] = true;
                endX[id] = touchInfo.x[i];
                endY[id] = touchInfo.y[i];

                data->diagOut->printf("touch id: %d, last: %lu (%d,%d)", touchInfo.id[i], millis() - last[id], touchInfo.x[i], touchInfo.y[i]);
            }

            for (uint8_t i = 0; i < 5; i++) {
                if(!detected[i] && down[i]) {
                    down[i] = false;
                    data->diagOut->printf("Touch up (%hu) start(%hu, %hu) end(%hu,%hu) ", i, startX[i], startY[i], endX[i], endY[i]);

                    Event event;
                    event.startX = startX[i];
                    event.startY = startY[i];
                    event.endX = endX[i];
                    event.endY = endY[i];

                    //Single touch
                    if((abs(startX[i]-endX[i]) < SINGLE_POINT_DISTANCE) && (abs(startY[i]-endY[i]) < SINGLE_POINT_DISTANCE)) {
                        data->diagOut->printf("Single touch at %d,%d", endX[i], endY[i]);
                        event.type = SINGLE_CLICK;
                    }
                    //Slide right
                    else if((abs(startX[i]-endX[i]) > SLIDE_ALONG_DISTANCE) && (abs(startY[i]-endY[i]) < SLIDE_ACROSS_DISTANCE) && (endX[i] > startX[i])) {
                        data->diagOut->printf("Slide right from %d,%d", startX[i], endY[i]);
                        event.type = SLIDE_LEFT;
                    }
                    //Slide left
                    else if((abs(startX[i]-endX[i]) > SLIDE_ALONG_DISTANCE) && (abs(startY[i]-endY[i]) < SLIDE_ACROSS_DISTANCE) && (endX[i] < startX[i])) {
                        data->diagOut->printf("Slide left from %d,%d", startX[i], endY[i]);
                    }
                    //Slide down
                    else if((abs(startX[i]-endX[i]) < SLIDE_ACROSS_DISTANCE) && (abs(startY[i]-endY[i]) > SLIDE_ALONG_DISTANCE) && (endY[i] > startY[i])) {
                        data->diagOut->printf("Slide down from %d,%d", startX[i], endY[i]);
                    }
                    //Slide up
                    else if((abs(startX[i]-endX[i]) < SLIDE_ACROSS_DISTANCE) && (abs(startY[i]-endY[i]) > SLIDE_ALONG_DISTANCE) && (endY[i] < startY[i])) {
                        data->diagOut->printf("Slide up from %d,%d", startX[i], endY[i]);
                    }
                    data->action(event);
                }
            }

            *(data->i2cBusy) = false;
        }
        delay(7);
    }
}

void GxFT5436::check(const char text[], TouchInfo& touchinfo)
{
  for (uint8_t i = 0; i < CFG_MAX_TOUCH_POINTS; i++)
  {
    DiagOut.print(text); DiagOut.print(" ("); DiagOut.print(touchinfo.x[i]); DiagOut.print(", "); DiagOut.print(touchinfo.y[i]); DiagOut.print(") ");
  }
  DiagOut.println(touchinfo.touch_count);
}

void GxFT5436::I2C_Write(uint8_t dev_addr, uint8_t reg_addr, uint8_t data)
{
  //DiagOut.println("I2C_Write");
  I2C.beginTransmission(dev_addr);
  I2C.write(reg_addr);
  I2C.write(data);
  I2C.endTransmission();
  //DiagOut.println("I2C_Write done");
}

uint8_t GxFT5436::I2C_Read(uint8_t dev_addr, uint8_t reg_addr)
{
  uint8_t data = 0;
  I2C.beginTransmission(dev_addr);
  I2C.write(reg_addr);
  I2C.endTransmission();
  I2C.requestFrom(dev_addr, uint8_t(1));
  if (I2C.available())
  {
    data = I2C.read();
  }
  return data;
}

void GxFT5436::I2C_Read(uint8_t dev_addr, uint8_t reg_addr, uint8_t* data, uint8_t n)
{
  I2C.beginTransmission(dev_addr);
  I2C.write(reg_addr);
  I2C.endTransmission();
  I2C.requestFrom(dev_addr, n);
  uint8_t i = 0;
  while (I2C.available())
  {
    data[i++] = I2C.read();
  }
  //DiagOut.print("I2C_Read "); DiagOut.println(i);
}


GxFT5436::TouchInfo::TouchInfo()
{
  clear();
}

void GxFT5436::TouchInfo::clear()
{
  touch_count = 0;
  for (uint8_t i = 0; i < CFG_MAX_TOUCH_POINTS; i++)
  {
    x[i] = 0;
    y[i] = 0;
  }
}

bool GxFT5436::TouchInfo::operator==(TouchInfo to)
{
  return ((touch_count == to.touch_count) &&
          (x[0] == to.x[0]) && (y[0] == to.y[0]) &&
          (x[1] == to.x[1]) && (y[1] == to.y[1]) &&
          (x[2] == to.x[2]) && (y[2] == to.y[2]) &&
          (x[3] == to.x[3]) && (y[3] == to.y[3]) &&
          (x[4] == to.x[4]) && (y[4] == to.y[4]));
}