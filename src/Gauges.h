#ifndef _GAUGES_H
#define _GAUGES_H

#include "Settings.h"
#include "Data.h"
#include "TFT_eSPI.h"
#include "Lock.h"
#include "Clickable.h"

#define SCALE_SPRITE_Y_OFFSET_12 2
#define SCALE_SPRITE_Y_OFFSET_16 3
#define GAUGES_LINE_SPACING 10

#define STATUS_WIFI_SYMBOL "a"
#define STATUS_BT_SYMBOL "b"
#define STATUS_HEADLIGHT_SYMBOL "c"
#define STATUS_THROTTLE_SYMBOL "d"

double calcX(int16_t startX, double deg, int16_t radius);
double calcY(int16_t startY, double deg, int16_t radius);
double rad(double);

enum StatusBar {
    WIFI, BT, HEADLIGHTS, THROTTLE, STATUS_BAR_SIZE
};

enum Side {
    LEFT, RIGHT, MID, TIME, STATUS_BAR, SIDE_SIZE
};

class Gauges {

public:
        TFT_eSPI *tft;
        SettingsClass::Field **gen;
        Lock *lock;

        SettingsClass::DataSource* selected;

        double arrR[91], arrOffset[91], arrX[91], arrY[91];
        TFT_eSprite* scaleSprite[2][5];
        TFT_eSprite* needleUpdate;
        TFT_eSprite* textUpdate;

        boolean redraw[SIDE_SIZE];
        int16_t selectedInfoCoords[4]; // x, y, w, h;
        bool selectedInfoVisible;
        ulong selectedInfoTimestamp;

        bool icons[STATUS_BAR_SIZE];

        void init(TFT_eSPI *t, Lock *l);
        void reInit();
        void fillTables();
        void createScaleSprites(Side side);
        void prepare();
        void clean();

        void drawScalePiece(void* target, bool isSprite, int deg, int side, int offsetX, int offsetY, int length, int width, uint16_t color);
        void drawScale(void* target, bool isSprite, int side, int offsetX, int offsetY, int w, int start, int end);
        void updateNeedle(int side);
        void updateText();
        void updateStatusBar();
        void drawSelectedInfo();
        void clearSelectedInfo();

        void setSelectedFromState();
        void setSelected(Side, SettingsClass::DataSource selected);
        void getSelected(SettingsClass::DataSource* selected);
        void cycleData(Side side);

        Clickable *leftGauge, *rightGauge, *midGauge, *date;
        std::vector<Clickable*> clickables;
        std::vector<Clickable*>* getClickables();

        static void processEvent(GxFT5436::Event, void*);

};


#endif
