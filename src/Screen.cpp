#include "Screen.h"

ScreenClass Screen;

void ScreenClass::processEvent(GxFT5436::Event event, void* param) {

    Serial.println(event.toString());

    if(event.type == SINGLE_CLICK) {

        auto params = (EventParams*)param;

        for(auto clickable : *(params->clickables)) {
            if (*(params->touchEnabled) && clickable->isVisible() &&
                clickable->isInBoundaries(event.x[0], event.y[0])) {
                clickable->onClick();
                break;
            }
        }

    } else if(event.type == TWO_POINT_CLICK) {

    }

}

void ScreenClass::init() {
    ledcSetup(0, 5000, 8);
    ledcAttachPin(32, 0);
    ledcWrite(0, 0);
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
        showPrompt("SSID: " + String((char *)Settings.general[WIFI_SSID]->getString().c_str()) +
        "\npass: " + String((char *)Settings.general[WIFI_PASS]->getString().c_str()) +
        "\nIP: " + WiFi.localIP().toString());
    });

    prompt = new Prompt;
    prompt->init(tft, lock, Networking.getServerOnPointer());
    prompt->setOnClick([this]() {
        if(prompt->isDismissible())
            switchView(previousView);
    });
    clickables.push_back(prompt);
    eventParams = {&clickables, &_touchEnabled, this};
    Data.touch.addOnEvent(processEvent, (void*)&eventParams);
}

void ScreenClass::reloadSettings() {
    reset = true;
}


View ScreenClass::getView() {
    return currentView;
}

void ScreenClass::switchView(View view) {
    lock->lock();

    previousView = currentView;
    currentView = view;

    for(auto clickable : clickables)
        clickable->setVisibility(false);

    switch(currentView) {
        case INIT: break;
        case GAUGES:  {
            for(auto & r : gauges->redraw)
                r = true;
            for(auto v : *gauges->getClickables()) {
                v->setVisibility(true);
            }
            break;
        }
        case CLOCK:  {
            break;
        }
        case PROMPT:  {
            prompt->setVisibility(true);
            break;
        }
    }

    tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());

    Log.logf_d("Current view: %d\n", currentView);
    lock->release();
}

unsigned long t, t1;
void ScreenClass::tick() {
    lock->lock();
//    Log.logf("pause: %s, paused: %s\n", _pause ? "true" : "false",  _paused ? "true" : "false");
    if(_pause) {
        _paused = true;
    } else {
        _paused = false;
        t = millis();
        if(reset) {
            tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
            gauges->reInit();
            prompt->reInit();
            reset = false;
        }
        switch(currentView) {
            case GAUGES:  {
                t1 = millis();
                gauges->updateText();
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
        }

#if defined(LOG_FRAMETIME) || defined(LOG_DETAILED_FRAMETIME)
        Log.logf("frametime: %lu\n", millis()-t);
#endif
    }
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

void ScreenClass::closePrompt() {
    if(currentView == PROMPT)
        switchView(previousView);
}

void ScreenClass::setBrightness(uint8_t x) {
    _brightness = x;
    ledcWrite(0, x);
}