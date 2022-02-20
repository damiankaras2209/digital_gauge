#include "Screen.h"

ScreenClass Screen;

uint16_t ScreenClass::c24to16(int i) {
    return tft->color24to16(i);
}

void ScreenClass::processEvent(GxFT5436::Event event, void* param) {

    Serial.println(event.toString());

    if(event.type == SINGLE_CLICK) {

        int16_t x = event.x;
        int16_t y = event.y;

        //        Log.logf("Single touch at %d,%d\n", x, y);

        switch (Screen.getView()) {
            case PROMPT: {
                //dismiss menu
                Screen.setGaugeMode();
                break;
            }
            case GAUGES: {
                //show menu
                if(Screen.getView() != PROMPT && x > Settings.general[WIDTH]->get<int>() / 2 - Settings.general[NEEDLE_CENTER_OFFSET]->get<int>() && x < Settings.general[WIDTH]->get<int>() / 2 + Settings.general[NEEDLE_CENTER_OFFSET]->get<int>() && y < Settings.general[HEIGHT]->get<int>() / 2) {
                    Screen.showMenu();
                    //                    Screen.showPrompt("SSID: " + String((char *)Settings.general[WIFI_SSID]->getString().c_str()) + "\npass: " + String((char *)Settings.general[WIFI_PASS]->getString().c_str()) + "\nIP: " + WiFi.localIP().toString() + "\nFW: " + getCurrentFirmwareVersionString());
                }

                //change gauge

                Side side = SIDE_LAST;
                if(x < Settings.general[WIDTH]->get<int>() / 2 - Settings.general[NEEDLE_CENTER_OFFSET]->get<int>())
                    side = LEFT;
                else if(x > Settings.general[WIDTH]->get<int>() / 2 + Settings.general[NEEDLE_CENTER_OFFSET]->get<int>())
                    side = RIGHT;
                else if(x > Settings.general[WIDTH]->get<int>() / 2 - Settings.general[NEEDLE_CENTER_OFFSET]->get<int>() &&
                x < Settings.general[WIDTH]->get<int>() / 2 + Settings.general[NEEDLE_CENTER_OFFSET]->get<int>() &&
                y > Settings.general[HEIGHT]->get<int>() / 2)
                    side = MID;

                if(side != SIDE_LAST) {
                    SettingsClass::DataSource selected[3];
                    Screen.gauges->getSelected(selected);

                    Log.logf("Current data: %s\n", Settings.dataSourceString[selected[side]].c_str());
                    do {
                        selected[side] = static_cast<SettingsClass::DataSource>(selected[side] + 1);
                        if(selected[side] == SettingsClass::LAST)
                            selected[side] = static_cast<SettingsClass::DataSource>(0);
                    } while (!Settings.general[DATA_BEGIN_BEGIN + selected[side] * DATA_SETTINGS_SIZE + DATA_ENABLE_OFFSET]->get<bool>());
                    Log.logf("Changing to: %s\n", Settings.dataSourceString[selected[side]].c_str());
                    Screen.gauges->setSelected(side, selected[side]);
                    Settings.saveSelected(selected);
                }
                break;
            }
            case MENU: {
                Menu::processEvent(event, Screen.menu);
                break;
            }
        }
    }

};

void ScreenClass::init() {
    tft = new TFT_eSPI;
    tft->init();
    tft->setRotation(3);
    tft->invertDisplay(1);
    lock = new Lock;
    gauges = new Gauges;
    menu = new Menu();
    menu->init(tft, lock);
    std::vector<Menu::Entry*> entries;
    entries.push_back(new Menu::Entry("ENTRY 1", []() {Log.logf("Fired entry %d\n", 1);}));
    entries.push_back(new Menu::Entry("ENTRY 2", []() {Log.logf("Fired entry %d\n", 2);}));
    entries.push_back(new Menu::Entry("ENTRY 3", []() {Log.logf("Fired entry %d\n", 3);}));
    entries.push_back(new Menu::Entry("ENTRY 4", []() {Log.logf("Fired entry %d\n", 4);}));
    entries.push_back(new Menu::Entry("ENTRY 5", []() {Log.logf("Fired entry %d\n", 5);}));
    entries.push_back(new Menu::Entry("ENTRY 6", []() {Log.logf("Fired entry %d\n", 6);}));
    entries.push_back(new Menu::Entry("ENTRY 7", []() {Log.logf("Fired entry %d\n", 7);}));
    menu->setEntries(entries);
    Data.touch.addOnEvent(processEvent, (void*)menu);
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

void ScreenClass::showMenu() {
    lock->lock();
    switchView(MENU);
    lock->release();
}

View ScreenClass::getView() {
    return currentView;
}

void ScreenClass::switchView(View view) {
//    if(currentView == GAUGES && view != GAUGES)
//        needleUpdate->unloadFont();
    if(currentView == MENU && view != MENU) {
        menu->clean();
    }
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
        case MENU:  {
            tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
            menu->prepare();
            menu->resetPosition();
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
        case MENU:  {
            menu->draw();
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
