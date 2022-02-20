#include "Menu.h"

#include <utility>


//Menu::Menu(int x, int y, int w, int h) : _x(x), _y(y), _w(w), _h(h) {}

void Menu::init(TFT_eSPI *t, Lock *l) {
    tft = t;
    lock = l;
    gen = Settings.general;
    menuSprite = new TFT_eSprite(tft);
    _w = gen[PROMPT_WIDTH]->get<int>();
    _h = gen[HEIGHT]->get<int>() - MENU_MARGIN * 2;
    _x = gen[WIDTH]->get<int>()/2 - _w / 2;
    _y = MENU_MARGIN;
    entryHeight = menuSprite->fontHeight() + 2 * PADDING + 2;
}

void Menu::setEntries(std::vector<Entry*> e) {
    entries = std::move(e);

    for(int i=0; i<entries.size(); i++) {
        entries.at(i)->setSize(_w, entryHeight);
        entries.at(i)->setPos(_x, _y + totalH);
        totalH += entries.at(i)->getHeight();
    }
    totalH += 1;
}


void Menu::prepare() {
    menuSprite->loadFont("GaugeHeavy12");
}

void Menu::clean() {
    menuSprite->unloadFont();
    menuSprite->deleteSprite();
}

void Menu::resetPosition() {
    prevScrollY = -1;
    scrollY = 0;
}

void Menu::scroll(int y) {
    lock->lock();
    scrollY += y;
    if(scrollY > 0)
        scrollY = 0;
//    Log.logf("posY + totalH: %d, gen[HEIGHT]->get<int>() - MENU_MARGIN: %d\n", posY + totalH, gen[HEIGHT]->get<int>() - MENU_MARGIN);
    if(scrollY < -abs(_h - totalH))
        scrollY = -abs(_h - totalH);
    for(int i=0; i<entries.size(); i++)
        entries.at(i)->setPos(_x, _y + scrollY + i * entries.at(i)->getHeight());
    lock->release();
}

void Menu::draw() {

    if(scrollY != prevScrollY) {

//        Log.logf("posY: %d, prevPosY: %d\n", scrollY, prevScrollY);
//        Log.logf("spriteW: %d, spriteH: %d, entryHeight: %d, entries.size(): %d, gen[WIDTH]->get<int>()/2: %d\n", _w, _h, entryHeight, entries.size(), gen[WIDTH]->get<int>() / 2);


        menuSprite->setColorDepth(8);
        menuSprite->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
        menuSprite->setTextDatum(CC_DATUM);
        menuSprite->createSprite(_w, _h, 1);
        menuSprite->fillSprite(gen[BACKGROUND_COLOR]->get<int>());

        for(int i=0; i<entries.size(); i++) {
            menuSprite->drawLine(0,
                             scrollY + i * entries.at(i)->getHeight(),
                             _w,
                             scrollY + i * entries.at(i)->getHeight(),
                             gen[FONT_COLOR]->get<int>());
            menuSprite->drawString(entries.at(i)->getText().c_str(), _w / 2, scrollY + i * entries.at(i)->getHeight() + entries.at(i)->getHeight() / 2);
        }
        menuSprite->drawLine(0,
                             scrollY + entries.size() * entries.back()->getHeight(),
                             _w,
                             scrollY + entries.size() * entries.back()->getHeight(),
                             gen[FONT_COLOR]->get<int>());
        menuSprite->pushSprite(_x, _y);
        menuSprite->deleteSprite();

        prevScrollY = scrollY;
    }
}

Menu::Entry* Menu::getEntryAt(int x, int y) {
    for(int i=0; i<entries.size(); i++) {
        if(entries.at(i)->isInBoundaries(x, y)) {
//            Log.logf("Matching entry: %d\n", i);
            return entries.at(i);
        }
    }
//    Log.log("Found none");
    return nullptr;
}

void Menu::processEvent(GxFT5436::Event e, void* menu) {
    Entry* entry = ((Menu*)menu)->getEntryAt(e.x, e.y);
    if(e.type == SINGLE_CLICK && entry != nullptr)
        entry->onClick();
}
