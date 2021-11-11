#ifndef _SCREEN_H_
#define _SCREEN_H_

#include <SPI.h>
#include <cmath>
#include <string>
#include <sstream>
#include <iomanip>

#include "Settings.h"
#include "Data.h"

#include "RTClib.h"
#include <../lib/TFT_eSPI-2.4.0-Beta/TFT_eSPI.h>
#include "ADS1X15.h"

float calcX(int16_t, int16_t, int16_t);
float calcY(int16_t, int16_t, int16_t);
double rad(int16_t);

enum Side {
    LEFT, RIGHT, MID, LAST
};

class Screen {

	protected:
		static Screen* screen;

	public:
		Screen();
		boolean shallWeReset;
		volatile boolean isBusy;
		static Screen *getInstance();
		void init(TFT_eSPI*, Data*, Settings::DataSource*);
		void reset();
		void blank();
		void updateNeedle(int, Settings::DataSource);
		void updateText(boolean, int fps);
		void showPrompt(String text);
		void addToPrompt(String text);
		uint16_t c24to16(int);

	private:
		TFT_eSPI *tft;
        Data *data;
        Settings *settings;
        Settings::DataSource *selected;
        volatile Settings::VisualSettings *vis;
		double arrR[91], arrX[91], arrY[91];
		bool promptShown = false;
		int lines = 0;
		void drawScalePiece(void*, boolean, int, int, int, int, int, int, int, int, int, uint16_t);
		void drawScale(void*, boolean, int, int, int, int, int, int);
		void fillTables();
		void setPromptFont();
		void drawNeedle();

	//void readSettings();

};

#endif