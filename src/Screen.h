#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "Log.h"

#include <SPI.h>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>

#include "Settings.h"
#include "Data.h"

#include "RTClib.h"
#include "TFT_eSPI.h"
#include "ADS1X15.h"

#define SCALE_SPRITE_Y_OFFSET_12 2
#define SCALE_SPRITE_Y_OFFSET_16 3
#define LINE_SPACING 10

float calcX(int16_t, int16_t, int16_t);
float calcY(int16_t, int16_t, int16_t);
double rad(int16_t);

enum Side {
    LEFT, RIGHT, MID, SIDE_LAST
};

enum View {
    GAUGES, CLOCK, PROMPT
};

class Screen {

	protected:
		static Screen* screen;

	public:
		Screen();
		volatile boolean isBusy;
		static Screen *getInstance();
		void init(TFT_eSPI*, Data*);
		void setSelected(Settings::DataSource*);
		void setSelected(Side, Settings::DataSource);
		void getSelected(Settings::DataSource*);
		void reset();
		void showPrompt(String text, int lineSpacing = LINE_SPACING, boolean useDefaultFont = false);
		void appendToPrompt(String text, int lineSpacing = LINE_SPACING, boolean useDefaultFont = false);
		void setClockMode();
		void setGaugeMode();
        View getView();
		void tick();

	private:
		TFT_eSPI *tft;
        Data *data;
        Settings *settings;
        Settings::DataSource selected[3];
        Settings::Field** gen;
		double arrR[91], arrX[91], arrY[91];
		TFT_eSprite* scaleSprite[2][5];
		TFT_eSprite* needleUpdate;
		TFT_eSprite* textUpdate;
		int lines = 0;
		volatile View currentView;
		boolean drawWhole[2];
		int16_t selectedInfoCoords[4]; // x, y, w, h;
		bool selectedInfoVisible;
		ulong selectedInfoTimestamp;
		uint16_t c24to16(int);
		void drawScalePiece(TFT_eSprite*, int, int, int, int, int, int, uint16_t);
		void drawScale(TFT_eSprite*, int, int, int, int, int, int);
		void updateNeedle(int);
		void updateText(boolean, int fps);
		void drawSelectedInfo();
		void clearSelectedInfo();
		void fillTables();
		void createScaleSprites(Side);
		void switchView(View);
		void lock();
		void release();



	//void readSettings();

};

#endif