#include "Screen.h"

ScreenClass Screen;

uint16_t ScreenClass::c24to16(int i) {
    return tft->color24to16(i);
}

void ScreenClass::init() {
    tft = new TFT_eSPI;
    tft->init();
    tft->setRotation(3);
    tft->invertDisplay(1);
    lock = new Lock;
    gauges = new Gauges;
    gauges->init(tft, lock);
    gen = Settings.general;
}

void ScreenClass::reset() {
    lock->lock();
    tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
    gauges->reset();
	lock->release();
}

void ScreenClass::setClockMode() {
    lock->lock();
    switchView(CLOCK);
    lock->release();
}
void ScreenClass::setGaugeMode() {
    lock->lock();
    switchView(GAUGES);
    lock->release();
}
View ScreenClass::getView() {
    return currentView;
}

void ScreenClass::switchView(View view) {
//    if(currentView == GAUGES && view != GAUGES)
//        needleUpdate->unloadFont();
    switch(view) {
        case GAUGES:  {
            tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
            gauges->drawWhole[0] = true;
            gauges->drawWhole[1] = true;
            gauges->updateText(true, 0);
            break;
        }
        case CLOCK:  {
            break;
        }
        case PROMPT:  {
            tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
            tft->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
            tft->drawRect(gen[WIDTH]->get<int>()/2 - gen[PROMPT_WIDTH]->get<int>()/2,
                          gen[HEIGHT]->get<int>()/2 - gen[PROMPT_HEIGHT]->get<int>()/2,
                          gen[PROMPT_WIDTH]->get<int>(),
                          gen[PROMPT_HEIGHT]->get<int>(),
                          gen[FONT_COLOR]->get<int>());
            tft->setTextDatum(CC_DATUM);
            break;
        }
    }
    currentView = view;
    Log.logf("Current view: %d", currentView);
}

unsigned long t, t1;
void ScreenClass::tick() {
    lock->lock();
    t = millis();
    switch(currentView) {
        case GAUGES:  {
            t1 = millis();
            gauges->updateText(false, 0);
#ifdef LOG_DETAILED_FRAMETIME
            Log.logf(" mid: %lu\n", millis()-t1);
            t1 = millis();
#endif
            gauges->updateNeedle(LEFT);
#ifdef LOG_DETAILED_FRAMETIME
            Log.logf(" left: %lu\n", millis()-t1);
            t1 = millis();
#endif
            gauges->updateNeedle(RIGHT);
#ifdef LOG_DETAILED_FRAMETIME
            Log.logf(" right: %lu ", millis()-t1);
#endif

            if(gauges->selectedInfoVisible && millis() - gauges->selectedInfoTimestamp > 5000)
                gauges->clearSelectedInfo();
            break;
        }
        case CLOCK:  {
            break;
        }
        case PROMPT:  {
            break;
        }
    }

#if defined(LOG_FRAMETIME) || defined(LOG_DETAILED_FRAMETIME)
    Log.logf("frametime: %lu\n", millis()-t);
#endif
    lock->release();
    delay(1);
}


void ScreenClass::showPrompt(String text, int lineSpacing, boolean useDefaultFont) {
    lock->lock();
    switchView(PROMPT);

    if(!useDefaultFont)
        tft->loadFont("GaugeHeavy12");

    std::string str = text.c_str();
//    Log.log(str.c_str());

    std::size_t nextLine = 0;
    lines = 0;

    while(nextLine != std::string::npos) {

//        Log.log(str.c_str());
//        Log.log(nextLine);

        nextLine = str.find_first_of('\n');
        tft->drawString(str.substr(0, nextLine).c_str(), gen[WIDTH]->get<int>()/2, gen[HEIGHT]->get<int>()/2-40+(lines++)*(tft->fontHeight()+lineSpacing));
        str = str.substr(nextLine+1);
    }
    lock->release();
}

void ScreenClass::appendToPrompt(String text, int lineSpacing, boolean useDefaultFont) {
    lock->lock();
    if(currentView != PROMPT) {
        Log.log("appendToPrompt() called without showPrompt()");
        return;
    }
    std::string str = text.c_str();
    std::size_t nextLine = 0;
    while(nextLine != std::string::npos) {
        nextLine = str.find_first_of('\n');
        tft->drawString(str.substr(0, nextLine).c_str(), gen[WIDTH]->get<int>()/2, gen[HEIGHT]->get<int>()/2-40+(lines++)*(tft->fontHeight()+lineSpacing));
        str = str.substr(nextLine+1);
    }
    lock->release();
}
