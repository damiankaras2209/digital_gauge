#ifndef _MENU_H
#define _MENU_H

#include <utility>

#include "TFT_eSPI.h"
#include "Settings.h"
#include "Lock.h"
#include "GxFT5436.h"
#include "Clickable.h"


#define LINE_SPACING 10
#define PADDING 20
#define MENU_HEIGHT 200
#define MENU_ENTRIES 4
#define MENU_MARGIN 70

class Menu {

    public:
        class Entry : public Clickable {
            String text;
            public:
                explicit Entry(String str, std::function<void()> f) {
                    text = std::move(str);
                    _onClick = std::move(f);
                };
                String getText() {
                    return text;
                };
        };

    private:

        TFT_eSPI *tft;
        SettingsClass::Field **gen;
        Lock *lock;
        bool* serverOn;

        int16_t _x = 0, _y = 0, _w = 0, _h = 0;
        int16_t prevScrollY = -1;
        int16_t scrollY = 0;
        int16_t entryHeight;
        int16_t totalH = 0;
        TFT_eSprite *menuSprite;

    public:
        std::vector<Entry*> entries;
        void init(TFT_eSPI *t, Lock *l, bool* serverOn);
        void setEntries(std::vector<Entry*> entries);
        void prepare();
        void clean();
        void resetPosition();
        void scroll(int);
        void draw();
        Entry* getEntryAt(int x, int y);
        static void processEvent(GxFT5436::Event, void*);


};


#endif
