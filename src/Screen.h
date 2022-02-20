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
#include "Gauges.h"
#include "Menu.h"
#include "Lock.h"

#define SCALE_SPRITE_Y_OFFSET_12 2
#define SCALE_SPRITE_Y_OFFSET_16 3

enum View {
	GAUGES, CLOCK, PROMPT, MENU
};

class ScreenClass {

	public:
        void init();
		void reset();
		void showPrompt(String text, int lineSpacing = LINE_SPACING, boolean useDefaultFont = false);
		void appendToPrompt(String text, int lineSpacing = LINE_SPACING, boolean useDefaultFont = false);
		void setClockMode();
		void setGaugeMode();
		void showMenu();
		View getView();
		void tick();

		Gauges *gauges;
		Menu *menu;

	private:
		TFT_eSPI *tft;
		SettingsClass::Field **gen;
		Lock *lock;
        volatile View currentView;
		int lines = 0;
		uint16_t c24to16(int);

		void switchView(View);

		static void processEvent(GxFT5436::Event, void*);


};

extern ScreenClass Screen;

#endif