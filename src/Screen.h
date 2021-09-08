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


class Screen {

	protected:
		static Screen* screen;

	public:
		Screen();
		boolean shallWeReset;
		volatile boolean isBusy;
		static Screen *getInstance();
		void init(TFT_eSPI*, Data*);
		void reset();
		void updateNeedle(int, float);
		void updateText(boolean, int fps);
		uint16_t c24to16(int);

	private:
		TFT_eSPI *tft;
        Data *data;
		double arrR[91], arrX[91], arrY[91];
		void drawScalePiece(void*, boolean, int, int, int, int, int, int, int, int, int, uint16_t);
		void drawScale(void*, boolean, int, int, int, int);
		void fillTables();
		void drawNeedle();

	//void readSettings();

};


#endif