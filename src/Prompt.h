#ifndef _PROMPT_H
#define _PROMPT_H

#include "TFT_eSPI.h"
#include "Settings.h"
#include "Lock.h"
#include "Clickable.h"

#define LINE_SPACING 5

class Prompt : public Clickable {

    TFT_eSPI *tft;
    SettingsClass::Field **gen;
    Lock *lock;
    bool* serverOn;

    String _text;
    int _lines = 0;
    int _lineSpacing = LINE_SPACING;
    bool _hasChanged = false;
    bool _useDefaultFont = false;
    bool _dismissible = true;
    TFT_eSprite *sprite;

public:

    void init(TFT_eSPI *t, Lock *l, bool* serverOn);
    void reInit();
    void setText(String text);
    void appendText(String text);
    void setLineSpacing(int spacing);
    void setUseDefaultFont(bool b);
    void setDismissible(bool b);
    bool isDismissible();
    void draw();

};


#endif