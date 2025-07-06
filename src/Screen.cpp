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
        auto params = (EventParams*)param;
        auto clickables =  *(params->clickables);

        if(*(params->touchEnabled) && clickables[LEFT]->isVisible() && clickables[RIGHT]->isVisible() &&
            (
                (
                clickables[LEFT]->isInBoundaries(event.x[0], event.y[0]) &&
                clickables[RIGHT]->isInBoundaries(event.x[1], event.y[1])
                ) || (
                clickables[LEFT]->isInBoundaries(event.x[1], event.y[1]) &&
                clickables[RIGHT]->isInBoundaries(event.x[0], event.y[0])
                )
           )
        ) {
            Log.logf("Throttle!\n", 3);
            if(millis() - Data.data.lastValveChange > THROTTLE_VALVE_DELAY) {
                params->screen->prompt->setDismissible(false);
                params->screen->showPrompt(String(Settings.state.throttleState ? "Closing" : "Opening") + " exhaust throttle valve");
                Data.data.shouldToggleValve = true;
                while(millis() - Data.data.lastValveChange > THROTTLE_VALVE_DELAY) {
                    delay(50);
                }
                params->screen->closePrompt();
                params->screen->prompt->setDismissible(true);
            }
            else {
                params->screen->showPrompt("Wait at least " + String(THROTTLE_VALVE_DELAY) + " seconds before\ntoggling exhaust throttle valve again");
            }

        }

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
        switchView(MENU);
    });


//    clickables.insert(gauges->getClickables()->end(), gauges->getClickables()->begin(), gauges->getClickables()->end());
    prompt = new Prompt;
    prompt->init(tft, lock, WebServer.getServerOnPointer());
    prompt->setOnClick([this]() {
        if(prompt->isDismissible()) {
            closePrompt();
        }
    });
    clickables.push_back(prompt);
    menu = new Menu();
    menu->init(tft, lock, WebServer.getServerOnPointer());
    std::vector<Menu::Entry*> entries;

    entries.push_back(new Menu::Entry("BACK", [this]() {
        switchView(GAUGES);
    }));
    // entries.push_back(new Menu::Entry("BLE TEST", [this]() {
    //     Networking.sendMessage(MESSAGE_GATE);
    // }));
    entries.push_back(new Menu::Entry("GATE", [this]() {
        HTTPClient http;
        http.begin(URL1);
        prompt->setDismissible(false);
        showPrompt("Sending gate request... ");
        if(WiFi.status() == WL_CONNECTED) {
            const int httpResponseCode = http.POST("");
            if (httpResponseCode == 200) {
                appendToPrompt("\nSuccess");
            } else {
                appendToPrompt("\nFail");
            }
        } else {
            appendToPrompt("\nNo connection");
        }
        prompt->setDismissible(true);
        std::thread t1([this](){
            delay(2000);
            closePrompt();
        });
        t1.detach();
    }));
    // String networkStr = "";
    // switch (Settings.state.networkType) {
    //     case NetworkingClass::BLE:
    //         networkStr = "BLE";
    //     case NetworkingClass::WIFI:
    //         networkStr = "WIFI";
    //     default:
    // }
    // entries.push_back(new Menu::Entry("NETWORK: " + networkStr, [this]() {
    //     switch (Settings.state.networkType) {
    //         case NetworkingClass::BLE:
    //             Settings.state.networkType = NetworkingClass::WIFI;
    //         case NetworkingClass::WIFI:
    //             Settings.state.networkType = NetworkingClass::BLE;
    //         default:
    //     }
    //     Settings.saveState();
    // }));
    entries.push_back(new Menu::Entry("SHOW INFO", [this]() {
        showPrompt("");
        prompt->setAutoRefresh(1000);
        prompt->setGetText([]() {
            return
                "SSID: " + String((char *)Settings.general[WIFI_SSID]->getString().c_str()) +
                "\npass: " + String((char *)Settings.general[WIFI_PASS]->getString().c_str()) +
                "\nIP: " + WiFi.localIP().toString() +
                "\nRSSI: " + WiFi.RSSI() + "dB" +
                "\nFW: " + Updater.firmware.toString() +" FS: " + Updater.filesystemCurrent.toString() +
                "\nMAC: " + Updater.getMac();
        });
    }));
    entries.push_back(new Menu::Entry("SYNC TIME", [this]() {
        prompt->setDismissible(false);
        showPrompt("Getting time from server... ");
        if(WiFi.status() == WL_CONNECTED) {
            switch (Data.adjustTime(&Data.data)) {
                case  D_SUCCESS: appendToPrompt("\nSuccess"); break;
                case  D_FAIL: appendToPrompt("\nFail"); break;
            }
        } else {
            appendToPrompt("\nNo connection");
            prompt->setDismissible(true);
        }
        prompt->setDismissible(true);
    }));
    entries.push_back(new Menu::Entry("POST", [this]() {
        showPrompt("POST:");
        for(int i=0; i<Device::D_LAST; i++)
            appendToPrompt("\n" + deviceName[i] + ": " + (Data.status[i] ? "good" : "fail"));
    }));
    entries.push_back(new Menu::Entry("RESET WIFI CREDENTIALS", [this]() {
        Settings.general[WIFI_SSID]->setDefault();
        Settings.general[WIFI_PASS]->setDefault();
        Settings.save(false);
        showPrompt("SSID: " + String((char *)Settings.general[WIFI_SSID]->getString().c_str()) +
             "\npass: " + String((char *)Settings.general[WIFI_PASS]->getString().c_str()));
    }));
    entries.push_back(new Menu::Entry("RESTART", [this]() {
        esp_restart();
    }));
    entries.push_back(new Menu::Entry("CHECK FOR UPDATE", [this]() {
        showPrompt("Checking for updates... ");
        prompt->setDismissible(false);
        if(WiFi.status() == WL_CONNECTED) {
            Updater.setOnSuccessCallback([this]() {
                Log.logf("Restarting in 5 seconds");
                appendToPrompt("\nRestarting in 5 seconds");
                Screen.tick();
                delay(5000);
                setBrightness(0);
                esp_restart();
            });
            Updater.setOnFinnish([this]() {
                prompt->setDismissible(true);
            });
            Updater.checkForUpdate([this](String str) {
                appendToPrompt(std::move(str));
                Screen.tick();
            });
        } else {
            appendToPrompt("\nNo connection");
            prompt->setDismissible(true);
        }
    }));
    menu->setEntries(entries);
    for (auto clickable: entries) {
        clickables.push_back(clickable);
    }
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

    if(previousView == MENU && currentView != MENU) {
        menu->clean();
    }

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
        case MENU:  {
            menu->prepare();
            menu->resetPosition();
            for(auto clickable : menu->entries)
                clickable->setVisibility(true);
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
            menu->reInit();
            prompt->reInit();
            reset = false;
        }
        switch(currentView) {
            case GAUGES:  {
                t1 = millis();
                gauges->updateText();
                gauges->updateStatusBar();
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
    }
    lock->release();
    delay(1);
}


void ScreenClass::showPrompt(String text, int lineSpacing, boolean useDefaultFont) {
    if(currentView != PROMPT)
        switchView(PROMPT);
    lock->lock();
    prompt->setText(std::move(text));
    prompt->setLineSpacing(lineSpacing);
    prompt->setUseDefaultFont(useDefaultFont);
//    prompt->draw();
    lock->release();
}

void ScreenClass::appendToPrompt(String text) {
    lock->lock();
    prompt->appendText(std::move(text));
//    draw()
    lock->release();
}

void ScreenClass::closePrompt() {
    if(currentView == PROMPT) {
        prompt->setAutoRefresh(0);
        switchView(previousView);
    }
}

void ScreenClass::setBrightness(uint8_t x) {
    _brightness = x;
    ledcWrite(0, x);
}

void ScreenClass::pause(bool b, bool wait) {
//    Log.logf("Pause");
    _pause = b;
    if(wait)
        while(_paused != b)
            delay(1);
}

void ScreenClass::enableTouch(bool b) {
    _touchEnabled = b;
}
