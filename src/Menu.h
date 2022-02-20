#ifndef _MENU_H
#define _MENU_H

#include "TFT_eSPI.h"
#include "Settings.h"
#include "Lock.h"
#include "GxFT5436.h"

#define LINE_SPACING 10
#define PADDING 20
#define MENU_HEIGHT 200
#define MENU_ENTRIES 4
#define
#define MENU_MARGIN 70

class Menu {

    public:
        class Entry {
            String text;
            int16_t _x = 0, _y = 0, _w = 0, _h = 0;
            std::function<void()> _onClick;
            public:
                Entry(String str, std::function<void()> action = nullptr) {
                    text = str;
                    _onClick = std::move(action);
                };
                void setPos(int x, int y) {
                    _x = x ;
                    _y = y;
                }
                void setSize(int w, int h) {
                    _w = w;
                    _h = h;
                }
                void setOnClick(std::function<void()> action) {
                    _onClick = std::move(action);
                }
                void onClick() {
                    _onClick();
                }
                int16_t getHeight() {
                    return _h;
                }
                int16_t getY() {
                    return _y;
                }
                String getText() {
                    return text;
                };
                bool isInBoundaries(int x, int y) {
//                    Log.logf("x: %d, _x: %d, _w: %d, y: %d, _y: %d, _h: %d\n", x, _x, _w, y, _y, _h);
                    return x > _x && x < (_x + _w) && y > _y && y < (_y + _h);
                }
        };

    private:

        TFT_eSPI *tft;
        SettingsClass::Field **gen;
        Lock *lock;

        int16_t _x = 0, _y = 0, _w = 0, _h = 0;
        std::vector<Entry*> entries;
        int16_t prevScrollY = -1;
        int16_t scrollY = 0;
        int16_t entryHeight;
        int16_t totalH = 0;
        TFT_eSprite *menuSprite;

    public:
        void init(TFT_eSPI *, Lock *);
        void setEntries(std::vector<Entry*>);
        void prepare();
        void clean();
        void resetPosition();
        void scroll(int);
        void draw();
        Entry* getEntryAt(int, int);
        static void processEvent(GxFT5436::Event, void*);


};


#endif
