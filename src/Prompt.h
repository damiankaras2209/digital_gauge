#ifndef _PROMPT_H
#define _PROMPT_H

#include "TFT_eSPI.h"
#include "Settings.h"
#include "Lock.h"
#include "Clickable.h"

#define PROMPT_LINE_SPACING 5

class Prompt : public Clickable {

    TFT_eSPI *tft;
    SettingsClass::Field **gen;
    Lock *lock;
    bool* serverOn;

    ulong _lastRefreshTime = 0;
    uint _autoRefresh = 0;
    std::function<String()> _getText;
    String _text;
    int _lines = 0;
    int _lineSpacing = PROMPT_LINE_SPACING;
    bool _hasChanged = false;
    bool _useDefaultFont = false;
    bool _dismissible = true;
    TFT_eSprite *sprite;

public:

    void init(TFT_eSPI *t, Lock *l, bool* serverOn);
    void reInit();
    void setGetText(std::function<String()> f);
    void setText(String text);
    void appendText(String text);
    void setLineSpacing(int spacing);
    void setUseDefaultFont(bool b);
    void setAutoRefresh(int ms);
    void setDismissible(bool b);
    bool isDismissible();
    void draw();

};


#endif