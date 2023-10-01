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
#include "Networking.h"

#include "Wifi.h"
#include "RTClib.h"
#include "TFT_eSPI.h"
#include "ADS1X15.h"
#include "Gauges.h"
#include "Menu.h"
#include "Lock.h"
#include "Updater.h"
#include "Prompt.h"

#define SCALE_SPRITE_Y_OFFSET_12 2
#define SCALE_SPRITE_Y_OFFSET_16 3

enum View {
    INIT, GAUGES, CLOCK, PROMPT, MENU
};

class ScreenClass {

	public:
        void init();
		void reloadSettings();
        void closePrompt();
		void switchView(View view);
		void showPrompt(String text, int lineSpacing = LINE_SPACING, boolean useDefaultFont = false);
		void appendToPrompt(String text);
		View getView();
		void tick();
		void setBrightness(uint8_t x);
        void pause(bool b, bool wait);
        void enableTouch(bool b);

		Gauges *gauges;
		Menu *menu;

        TFT_eSPI *tft;

	private:
		SettingsClass::Field **gen;
		Lock *lock;
        volatile View previousView = INIT;
        volatile View currentView = INIT;
        uint8_t _brightness;
        volatile bool reset = false;
        volatile bool _pause = false;
        volatile bool _paused = false;
        bool _touchEnabled = true;


		std::vector<Clickable*> clickables;
		Prompt *prompt;

        typedef struct EventParams {
            std::vector<Clickable*> *clickables;
            bool *touchEnabled;
            ScreenClass* screen;
        } EventParams;
        EventParams eventParams;
		static void processEvent(GxFT5436::Event, void*);


};

extern ScreenClass Screen;

#endif