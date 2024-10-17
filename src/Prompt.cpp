//
// Created by Musztarda on 22.02.2022.
//

#include "Prompt.h"

void Prompt::init(TFT_eSPI *t, Lock *l, bool* b) {
    tft = t;
    lock = l;
    serverOn = b;
    gen = Settings.general;
    sprite = new TFT_eSprite(tft);
    sprite->setTextDatum(CC_DATUM);
    reInit();
}

void Prompt::reInit() {
    sprite->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
    _w = gen[PROMPT_WIDTH]->get<int>();
    _h = gen[PROMPT_HEIGHT]->get<int>();
    _x = gen[WIDTH]->get<int>()/2 - _w/2 + gen[OFFSET_X]->get<int>();
    _y = gen[HEIGHT]->get<int>()/2 - _h/2  + gen[OFFSET_Y]->get<int>();
}

void Prompt::setText(String t) {
    _text = std::move(t);
    _hasChanged = true;
}

void Prompt::appendText(String t) {
    _text += t;
    _hasChanged = true;
}

void Prompt::setLineSpacing(int i) {
    _lineSpacing = i;
    _hasChanged = true;
}

void Prompt::setUseDefaultFont(bool b) {
    _useDefaultFont = b;
    _hasChanged = true;
}

void Prompt::setDismissible(bool b ) {
    _dismissible = b;
}

bool Prompt::isDismissible() {
    return _dismissible;
}

void Prompt:: draw() {
    if(_hasChanged) {
        _hasChanged = false;
        if(!_useDefaultFont)
            sprite->loadFont("GaugeHeavy12");

        std::string str = _text.c_str();
        //    Log.logf(str.c_str());

        std::size_t nextLine = 0;
        _lines = 1;

        sprite->setColorDepth(8);
        if(!sprite->createSprite(_w, _h)) {
            Log.logf("Unable to create 8bit prompt sprite");
            sprite->setColorDepth(1);
            sprite->setBitmapColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
            sprite->createSprite(_w, _h);
        }

        sprite->drawRect(0, 0, _w, _h, gen[FONT_COLOR]->get<int>());
        while(nextLine != std::string::npos) {

            //        Log.logf(str.c_str());
            //        Log.logf(nextLine);

            nextLine = str.find_first_of('\n');
            sprite->drawString(str.substr(0, nextLine).c_str(), _w/2, (_lines++) * (sprite->fontHeight() + _lineSpacing));
            str = str.substr(nextLine+1);
        }

        sprite->pushSprite(_x, _y);
        sprite->deleteSprite();
    }
}
