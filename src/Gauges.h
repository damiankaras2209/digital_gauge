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
#include "Clickable.h"

#define SCALE_SPRITE_Y_OFFSET_12 2
#define SCALE_SPRITE_Y_OFFSET_16 3
#define LINE_SPACING 10

float calcX(int16_t, int16_t, int16_t);
float calcY(int16_t, int16_t, int16_t);
double rad(int16_t);

enum Side {
    LEFT, RIGHT, MID, TIME, SIDE_LAST
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

        boolean redraw[SIDE_LAST];
        int16_t selectedInfoCoords[4]; // x, y, w, h;
        bool selectedInfoVisible;
        ulong selectedInfoTimestamp;

        void init(TFT_eSPI *t, Lock *l);
        void reloadSettings();
        void fillTables();
        void createScaleSprites(Side side);
        void prepare();
        void clean();

        void drawScalePiece(void* target, bool isSprite, int deg, int side, int offsetX, int offsetY, int length, int width, uint16_t color);
        void drawScale(void* target, bool isSprite, int side, int offsetX, int offsetY, int w, int start, int end);
        void updateNeedle(int side);
        void updateText();
        void drawSelectedInfo();
        void clearSelectedInfo();

        void setSelected(SettingsClass::DataSource* selected);
        void setSelected(Side, SettingsClass::DataSource selected);
        void getSelected(SettingsClass::DataSource* selected);
        void cycleData(Side side);

        Clickable *leftGauge, *rightGauge, *midGauge, *date;
        std::vector<Clickable*> clickables;
        std::vector<Clickable*>* getClickables();

        static void processEvent(GxFT5436::Event, void*);

};


#endif
