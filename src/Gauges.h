#ifndef _GAUGES_H
#define _GAUGES_H

#include <SPI.h>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>

#include "Settings.h"
#include "Data.h"

#include "TFT_eSPI.h"
#include "Lock.h"

#define SCALE_SPRITE_Y_OFFSET_12 2
#define SCALE_SPRITE_Y_OFFSET_16 3
#define LINE_SPACING 10

float calcX(int16_t, int16_t, int16_t);
float calcY(int16_t, int16_t, int16_t);
double rad(int16_t);

enum Side {
    LEFT, RIGHT, MID, SIDE_LAST
};

class Gauges {

public:
        TFT_eSPI *tft;
        SettingsClass::Field **gen;
        Lock *lock;

        SettingsClass::DataSource selected[3];

        double arrR[91], arrOffset[91], arrX[91], arrY[91];
        TFT_eSprite* scaleSprite[2][5];
        TFT_eSprite* needleUpdate;
        TFT_eSprite* textUpdate;

        boolean drawWhole[2];
        int16_t selectedInfoCoords[4]; // x, y, w, h;
        bool selectedInfoVisible;
        ulong selectedInfoTimestamp;

        void init(TFT_eSPI *, Lock *);
        void reset();
        void drawScalePiece(TFT_eSprite*, int, int, int, int, int, int, uint16_t);
        void drawScale(TFT_eSprite*, int, int, int, int, int, int);
        void updateNeedle(int);
        void updateText(boolean, int fps);
        void drawSelectedInfo();
        void clearSelectedInfo();
        void fillTables();
        void createScaleSprites(Side);

        void setSelected(SettingsClass::DataSource*);
        void setSelected(Side, SettingsClass::DataSource);
        void getSelected(SettingsClass::DataSource*);


};


#endif
