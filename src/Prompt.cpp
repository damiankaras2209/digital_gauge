//
// Created by Musztarda on 22.02.2022.
//

#include "Prompt.h"

void Prompt::init(TFT_eSPI *t, Lock *l) {
    tft = t;
    lock = l;
    gen = Settings.general;
    sprite = new TFT_eSprite(tft);
    sprite->setColorDepth(8);
    sprite->setTextDatum(CC_DATUM);
    sprite->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
    _w = gen[PROMPT_WIDTH]->get<int>();
    _h = gen[PROMPT_HEIGHT]->get<int>();
    _x = gen[WIDTH]->get<int>()/2 - _w / 2;
    _y = gen[HEIGHT]->get<int>()/2 - _h / 2;
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
        //    Log.log(str.c_str());

        std::size_t nextLine = 0;
        _lines = 1;


        sprite->createSprite(_w, _h, 1);
        sprite->drawRect(0, 0, _w, _h, gen[FONT_COLOR]->get<int>());

        while(nextLine != std::string::npos) {

            //        Log.log(str.c_str());
            //        Log.log(nextLine);

            nextLine = str.find_first_of('\n');
            sprite->drawString(str.substr(0, nextLine).c_str(), _w/2, (_lines++) * (tft->fontHeight() + _lineSpacing));
            str = str.substr(nextLine+1);
        }

        sprite->pushSprite(_x, _y);
        sprite->deleteSprite();
    }
}
