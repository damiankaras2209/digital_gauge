#include "Screen.h"

ScreenClass Screen;

void ScreenClass::processEvent(GxFT5436::Event event, void* param) {

    Serial.println(event.toString());

    if(event.type == SINGLE_CLICK) {

        auto clickables = (std::vector<Clickable*>*)param;

        for(auto clickable : *clickables)
            if(clickable->isVisible() && clickable->isInBoundaries(event.x, event.y)) {
                clickable->onClick();
                break;
            }

    }

};

void ScreenClass::init() {
    gen = Settings.general;
    tft = new TFT_eSPI;
    tft->init();
    tft->setRotation(3);
    tft->invertDisplay(1);
    lock = new Lock;
    gauges = new Gauges;
    gauges->init(tft, lock);
    auto gaugesClickables = gauges->getClickables();
    for(auto v : *gaugesClickables) {
        clickables.push_back(v);
    }

    gaugesClickables->at(LEFT)->setOnClick([this]() {
        gauges->cycleData(LEFT);
    });
    gaugesClickables->at(RIGHT)->setOnClick([this]() {
        gauges->cycleData(RIGHT);
    });
    gaugesClickables->at(MID)->setOnClick([this]() {
        gauges->cycleData(MID);
    });
    gaugesClickables->at(TIME)->setOnClick([this]() {
        switchView(MENU);
    });


//    clickables.insert(gauges->getClickables()->end(), gauges->getClickables()->begin(), gauges->getClickables()->end());
    prompt = new Prompt;
    prompt->init(tft, lock);
    prompt->setOnClick([this]() {
        switchView(MENU);
    });
    clickables.push_back(prompt);
    menu = new Menu();
    menu->init(tft, lock);
    std::vector<Menu::Entry*> entries;
    entries.push_back(new Menu::Entry("BACK", [this]() {
        Log.logf("Fired entry %d\n", 1);
        switchView(GAUGES);
    }));
    entries.push_back(new Menu::Entry("SHOW INFO", [this]() {
        Log.logf("Fired entry %d\n", 2);
        showPrompt("SSID: " + String((char *)Settings.general[WIFI_SSID]->getString().c_str()) + "\npass: " + String((char *)Settings.general[WIFI_PASS]->getString().c_str()) + "\nIP: " + WiFi.localIP().toString() + "\nFW: " + getCurrentFirmwareVersionString() + " FS: " + getCurrentFilesystemVersionString());
    }));
    entries.push_back(new Menu::Entry("SYNC TIME", [this]() {
        Log.logf("Fired entry %d\n", 3);
        showPrompt("Getting time from server... ");
        if(WiFi.status() == WL_CONNECTED) {
            switch (Data.adjustTime(&Data.data)) {
                case  D_SUCCESS: appendToPrompt("\nSuccess"); break;
                case  D_FAIL: appendToPrompt("\nFail"); break;
            }
        } else {
            appendToPrompt("\nNo connection");
        }

    }));
    entries.push_back(new Menu::Entry("CHECK FOR UPDATE", [this]() {
        Log.logf("Fired entry %d\n", 4);
        Updater.checkForUpdate();
    }));
    menu->setEntries(entries);
    for (auto clickable: entries) {
        clickables.push_back(clickable);
    }
    Data.touch.addOnEvent(processEvent, (void*)&clickables);
}

void ScreenClass::reset() {
    lock->lock();
    tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
    gauges->reset();
	lock->release();
}


View ScreenClass::getView() {
    return currentView;
}

void ScreenClass::switchView(View view) {
    lock->lock();
//    if(currentView == GAUGES && view != GAUGES)
//        needleUpdate->unloadFont();
    for(auto clickable : clickables)
        clickable->setVisibility(false);

    if(currentView == MENU && view != MENU) {
        menu->clean();
    }

    switch(view) {
        case GAUGES:  {
            tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
            gauges->drawWhole[0] = true;
            gauges->drawWhole[1] = true;
            for(auto v : *gauges->getClickables()) {
                v->setVisibility(true);
            }
            gauges->updateText(true, 0);
            break;
        }
        case CLOCK:  {
            break;
        }
        case PROMPT:  {
            tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
            prompt->setVisibility(true);
            break;
        }
        case MENU:  {
            tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
            menu->prepare();
            menu->resetPosition();
            for(auto clickable : menu->entries)
                clickable->setVisibility(true);
            break;
        }
    }
    currentView = view;
    Log.logf("Current view: %d\n", currentView);
    lock->release();
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
            prompt->draw();
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
    if(currentView != PROMPT)
        switchView(PROMPT);
    lock->lock();
    prompt->setText(text);
    prompt->setLineSpacing(lineSpacing);
    prompt->setUseDefaultFont(useDefaultFont);
//    prompt->draw();
    lock->release();
}

void ScreenClass::appendToPrompt(String text) {
    lock->lock();
    prompt->appendText(text);
//    draw()
    lock->release();
}
