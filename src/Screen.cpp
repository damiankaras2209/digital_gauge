#include "Screen.h"

double rad(int16_t deg) {
    return 1.0*deg * PI / 180;
}

float calcX(int16_t startX, int16_t deg, int16_t radius) {
    return startX + radius * cos(rad(deg));
}

float calcY(int16_t startY, int16_t deg, int16_t radius) {
    return startY + radius * sin(rad(deg));
}

Screen* Screen::screen = nullptr;

Screen::Screen() {
    isBusy = false;
    settings = Settings::getInstance();
    gen = settings->general;
}

Screen* Screen::getInstance()
{
    if(screen==nullptr)
        screen = new Screen();
    return screen;
}

void Screen::lock() {
    while(isBusy)
        delay(1);
    isBusy = true;
}

void Screen::release() {
    isBusy = false;
}

uint16_t Screen::c24to16(int i) {
    return tft->color24to16(i);
}

void Screen::fillTables() {
	int density = 1;
	int a = gen[ELLIPSE_A]->get<int>();
	int b = gen[ELLIPSE_B]->get<int>();
	int offset = gen[NEEDLE_CENTER_OFFSET]->get<int>();;
	for(int i=0; i<91; i++) {
		double tga = tan(rad(i));
		double r = 0.0;

		if(i < 45) {
			for(int nx=1; nx<(a+1)*density; nx++) {
				double x = 1.0/density*nx;
				double y=tga*x;
				//tft->drawPixel(gen[WIDTH]->get<int>()/2+gen[NEEDLE_CENTER_OFFSET]->get<int>()+x,gen[HEIGHT]->get<int>()/2+y, TFT_RED);
				if(1.0*(x+offset)*(x+offset)/(a*a)+1.0*y*y/(b*b)> 0.98) {
					//tft->drawPixel(gen[WIDTH]->get<int>()/2+gen[NEEDLE_CENTER_OFFSET]->get<int>()+x,gen[HEIGHT]->get<int>()/2+y, TFT_WHITE);
					arrX[i] = x;
					arrY[i] = y;
					arrR[i] = sqrt(x*x+y*y);
					break;
				}
			}
		} else {
			for(int ny=1; ny<(b+1)*density; ny++) {
				double y = 1.0/density*ny;
				double x=y/tga;
				//tft->drawPixel(gen[WIDTH]->get<int>()/2+gen[NEEDLE_CENTER_OFFSET]->get<int>()+x,gen[HEIGHT]->get<int>()/2+y, TFT_RED);
				if(1.0*(x+offset)*(x+offset)/(a*a)+1.0*y*y/(b*b)> 0.98) {
					//tft->drawPixel(gen[WIDTH]->get<int>()/2+gen[NEEDLE_CENTER_OFFSET]->get<int>()+x,gen[HEIGHT]->get<int>()/2+y, TFT_WHITE);
					
					arrX[i] = x;
					arrY[i] = y;
					arrR[i] = sqrt(x*x+y*y);
					break;
				}
			}
		}
	}

}

//ulong t4;
void Screen::createScaleSprites(Side side) {
//    t4 = millis();
    if(side != MID) {
        for(int j=0; j<5; j++) {
            int start = settings->general[DATA_0 + selected[side] * DATA_SETTINGS_SIZE + DATA_SCALE_START_OFFSET]->get<int>();
            int end = settings->general[DATA_0 + selected[side] * DATA_SETTINGS_SIZE + DATA_SCALE_END_OFFSET]->get<int>();

            String string = (String)(start + j*(end-start)/gen[SCALE_TEXT_STEPS]->get<int>());

            int w,h;
            scaleSprite[side][j]->deleteSprite();
            scaleSprite[side][j]->setColorDepth(8);
            scaleSprite[side][j]->loadFont("GaugeHeavyNumbers12");
            w = scaleSprite[side][j]->textWidth(string);
            h = scaleSprite[side][j]->fontHeight();
            scaleSprite[side][j]->createSprite(w, h + SCALE_SPRITE_Y_OFFSET_12, 1);
            scaleSprite[side][j]->setTextColor(gen[FONT_COLOR]->get<int>());
            scaleSprite[side][j]->setTextDatum(TL_DATUM);
            scaleSprite[side][j]->drawString(string, 0, SCALE_SPRITE_Y_OFFSET_12);
            scaleSprite[side][j]->unloadFont();
            delay(1);
        }
    }
//    Log.logf("Sprites creation time: %lu", millis()-t4);
}

void Screen::init(TFT_eSPI *t, Data *d) {
	tft = t;
	data = d;
	for(int i=0; i<2; i++) {
	    for(int j=0; j<5; j++) {
	        scaleSprite[i][j] = new TFT_eSprite(tft);
	    }
	}
	needleUpdate = new TFT_eSprite(tft);
	needleUpdate->loadFont("GaugeHeavyNumbers12");
	textUpdate = new TFT_eSprite(tft);
	textUpdate->loadFont("GaugeHeavy16");
}

void Screen::setSelected(Settings::DataSource *s) {
   for(int i=LEFT; i<SIDE_LAST; i++) {
       selected[i] = s[i];
   }
}

void Screen::setSelected(Side side, Settings::DataSource s) {
    if(selected[side] != s) {
        lock();
        selected[side] = s;
        if(side != MID) {
            drawWhole[side] = true;
            createScaleSprites(side);
        }
        drawSelectedInfo();
        release();
    }
}

void Screen::getSelected(Settings::DataSource* s) {
    for(int i=LEFT; i<SIDE_LAST; i++)
        s[i] = selected[i];
}

void Screen::reset() {
    lock();
    tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
    fillTables();
    createScaleSprites(LEFT);
    createScaleSprites(RIGHT);
    selectedInfoCoords[2] = (gen[NEEDLE_CENTER_OFFSET]->get<int>() - gen[SCALE_TEXT_STEPS]->get<int>()) * 2; //width
    selectedInfoCoords[3] = (textUpdate->fontHeight()+LINE_SPACING)*4 - LINE_SPACING + SCALE_SPRITE_Y_OFFSET_16; //height
    selectedInfoCoords[0] = gen[WIDTH]->get<int>()/2 + gen[OFFSET_X]->get<int>() - selectedInfoCoords[2]/2; //x
    selectedInfoCoords[1] = gen[HEIGHT]->get<int>()/2 + gen[OFFSET_Y]->get<int>() + 5; //y
	drawWhole[0] = true;
	drawWhole[1] = true;
    updateText(true, 0);
	release();
}

void Screen::setClockMode() {
    lock();
    switchView(CLOCK);
    release();
}
void Screen::setGaugeMode() {
    lock();
    switchView(GAUGES);
    release();
}
View Screen::getView() {
    return currentView;
}

void Screen::switchView(View view) {
//    if(currentView == GAUGES && view != GAUGES)
//        needleUpdate->unloadFont();
    switch(view) {
        case GAUGES:  {
            tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
            drawWhole[0] = true;
            drawWhole[1] = true;
            updateText(true, 0);
            break;
        }
        case CLOCK:  {
            break;
        }
        case PROMPT:  {
            tft->fillScreen(gen[BACKGROUND_COLOR]->get<int>());
            tft->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
            tft->drawRect(gen[WIDTH]->get<int>()/2 - gen[PROMPT_WIDTH]->get<int>()/2,
                          gen[HEIGHT]->get<int>()/2 - gen[PROMPT_HEIGHT]->get<int>()/2,
                          gen[PROMPT_WIDTH]->get<int>(),
                          gen[PROMPT_HEIGHT]->get<int>(),
                          gen[FONT_COLOR]->get<int>());
            tft->setTextDatum(CC_DATUM);
            break;
        }
    }
    currentView = view;
    Log.logf("Current view: %d", currentView);
}

unsigned long t, t1;
void Screen::tick() {
    lock();
    t = millis();
    switch(currentView) {
        case GAUGES:  {
            t1 = millis();
            Screen::getInstance()->updateText(false, 0);
#ifdef LOG_DETAILED_FRAMETIME
            Log.logf(" mid: %lu\n", millis()-t1);
            t1 = millis();
#endif
            Screen::getInstance()->updateNeedle(LEFT);
#ifdef LOG_DETAILED_FRAMETIME
            Log.logf(" left: %lu\n", millis()-t1);
            t1 = millis();
#endif
            Screen::getInstance()->updateNeedle(RIGHT);
#ifdef LOG_DETAILED_FRAMETIME
            Log.logf(" right: %lu ", millis()-t1);
#endif

            if(selectedInfoVisible && millis() - selectedInfoTimestamp > 5000)
                clearSelectedInfo();
            break;
        }
        case CLOCK:  {
            break;
        }
        case PROMPT:  {
            break;
        }
    }

#if defined(LOG_FRAMETIME) || defined(LOG_DETAILED_FRAMETIME)
    Log.logf("frametime: %lu\n", millis()-t);
#endif
    release();
    delay(1);
}

void Screen::drawScalePiece(TFT_eSprite* c, int deg, int side, int spriteX, int spriteY, int length, int width, uint16_t color) {

    if(!side)
        side = -1;

    int m = 1;
    if(deg < 90) {
        deg = 90-deg;
    } else {
        deg-=90;
        m = -1;
    }

    //Log.log(deg);

    double x1 = arrX[deg];
    double y1 = arrY[deg];
    double x2 = sqrt((arrR[deg]-length)*(arrR[deg]-length)/(1+tan(rad(deg))*tan(rad(deg))));
    double y2 = tan(rad(deg))*x2;

    //if(deg <1) {

    //tft->drawPixel(gen[WIDTH]->get<int>()/2+gen[NEEDLE_CENTER_OFFSET]->get<int>()+x1,gen[HEIGHT]->get<int>()/2+y1, TFT_PINK);
    //tft->drawPixel(gen[WIDTH]->get<int>()/2+gen[NEEDLE_CENTER_OFFSET]->get<int>()+x2,gen[HEIGHT]->get<int>()/2+y2, TFT_BLUE);


    // Log.log(x1);
    // Log.log(" ");
    // Log.log(y1);
    // Log.log(" ");
    // Log.log(x2);
    // Log.log(" ");
    // Log.log(y2);

    c->drawWideLine(
            gen[WIDTH]->get<int>()/2 +side*(gen[NEEDLE_CENTER_OFFSET]->get<int>()+x1) - spriteX,
            gen[HEIGHT]->get<int>()/2 +m*(y1) - spriteY,
            gen[WIDTH]->get<int>()/2 +side*(gen[NEEDLE_CENTER_OFFSET]->get<int>()+x2) - spriteX,
            gen[HEIGHT]->get<int>()/2 +m*(y2) - spriteY,
            width,
            color,
            gen[BACKGROUND_COLOR]->get<int>());
    // }
}

unsigned long t3;

void Screen::drawScale(TFT_eSprite* c, int side, int spriteX, int spriteY, int w, int start, int end) {

    t3 = millis();

    float stepSmall = 180/((gen[SCALE_LARGE_STEPS]->get<int>()*gen[SCALE_SMALL_STEPS]->get<int>())*1.0);
    int steps = gen[SCALE_LARGE_STEPS]->get<int>()*gen[SCALE_SMALL_STEPS]->get<int>()+1;
    for(uint8_t i=0; i<steps; i++) {
        int16_t deg = stepSmall*i;
        drawScalePiece(c, deg, side, spriteX, spriteY,
                       (i%(steps/gen[SCALE_LARGE_STEPS]->get<int>())==0) ? gen[SCALE_LARGE_LENGTH]->get<int>() : gen[SCALE_SMALL_LENGTH]->get<int>(),
                       (i%(steps/gen[SCALE_LARGE_STEPS]->get<int>())==0) ? gen[SCALE_LARGE_WIDTH]->get<int>() : gen[SCALE_SMALL_WIDTH]->get<int>(),
                       (i%gen[SCALE_ACC_COLOR_EVERY]->get<int>()==0) ? gen[SCALE_ACC_COLOR]->get<int>() : gen[SCALE_COLOR]->get<int>());
    }

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf("scale pieces: %lu", millis()-t3);
    t3 = millis();
#endif

    for(int i=0; i<5; i++) {
        int deg;

        switch(i) {
            case 0: deg = 90;break;
            case 1: deg = 45;break;
            case 2: deg = 0;break;
            case 3: deg = 45;break;
            case 4: deg = 90;break;
        }

        int top = i>2 ? -1 : 1;

        scaleSprite[side][i]->pushToSprite(c,
               gen[WIDTH]->get<int>()/2 + (side ? 1 : -1)*(gen[NEEDLE_CENTER_OFFSET]->get<int>() + calcX(0, deg, arrR[deg] - gen[SCALE_TEXT_OFFSET]->get<int>())) - spriteX - scaleSprite[side][i]->width()/2,
               gen[HEIGHT]->get<int>()/2 + top*(calcY(0, deg, arrR[deg] - gen[SCALE_TEXT_OFFSET]->get<int>())) - spriteY - scaleSprite[side][i]->height()/2
               );

    }

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf(" draw numbers: %lu", millis()-t3);
    t3 = millis();
#endif
}

//static const uint16_t pallete[] = {
//	TFT_BLACK,		//  0  ^
// 	TFT_GREEN,   	//  1  |
//  	TFT_RED,   		//  2  |
//  	TFT_DARKGREY,   //  3  |
//	TFT_BLUE		//	4  |
//};

int xx = 0;

int pX1[2], pY1[2], pW[2], pH[2], pDeg[2];
Settings::DataSource pSource[2];

unsigned long t2;

void Screen::updateNeedle(int side) {

    t2 = millis();

    int start, end;
    float value;

    value = settings->general[DATA_0 + selected[side] * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->get<float>();
    start = settings->general[DATA_0 + selected[side] * DATA_SETTINGS_SIZE + DATA_SCALE_START_OFFSET]->get<int>();
    end = settings->general[DATA_0 + selected[side] * DATA_SETTINGS_SIZE + DATA_SCALE_END_OFFSET]->get<int>();

//    value = (sin(xx/PI/18)/2+0.5)*(end-start)+start;

    float val = (value-start)/(end-start);

	if(val<0.0)
	    val = 0.0;
	if(val>1.0)
	    val = 1.0;

	double deg = (1.0-val)*180;
	// Log.log(pos);
	// Log.log(" ");
	// Log.log(deg);
	deg-=90.0;
	// Log.log(" ");
	// Log.log(deg);

	//Log.log(deg);
	// Log.log(" ");
	// Log.log(length);

	double length;
	int x, y, x1, y1, w, h;

	int off = 5+gen[NEEDLE_TOP_WIDTH]->get<int>();

	length = arrR[lround(abs(deg))];

	if(deg >= 0) {
		x = length*cos(rad(deg));
		y = length*sin(rad(deg));
		y1 = gen[HEIGHT]->get<int>()/2-gen[NEEDLE_CENTER_RADIUS]->get<int>();
		h = max(y+gen[NEEDLE_CENTER_RADIUS]->get<int>(), gen[NEEDLE_CENTER_RADIUS]->get<int>()*2)+off;
//		tft->drawRect(x1, y1, w, h, TFT_RED);
	} else {
		y = length*cos(rad(deg+90));
		x = length*sin(rad(deg+90));
		y1 = gen[HEIGHT]->get<int>()/2-max((int)gen[NEEDLE_CENTER_RADIUS]->get<int>(), y)-off;
		h = max(y+gen[NEEDLE_CENTER_RADIUS]->get<int>(), gen[NEEDLE_CENTER_RADIUS]->get<int>()*2)+off;
//		tft->drawRect(x1, y1, w, h, TFT_RED);
	}

	x1 = gen[WIDTH]->get<int>()/2+(gen[NEEDLE_CENTER_OFFSET]->get<int>()-gen[NEEDLE_CENTER_RADIUS]->get<int>());
	w = max(x+gen[NEEDLE_CENTER_RADIUS]->get<int>(), gen[NEEDLE_CENTER_RADIUS]->get<int>()*2)+off;
	w = gen[ELLIPSE_A]->get<int>()-gen[NEEDLE_CENTER_OFFSET]->get<int>()+gen[NEEDLE_CENTER_RADIUS]->get<int>();

	// Log.log("x1:");
	// Log.log(x1);
	// Log.log(" pX1:");
	// Log.log(pX1[side]);
	// Log.log(" y1:");
	// Log.log(y1);
	// Log.log(" pY1:");
	// Log.log(pY1[side]);
	// Log.log(" w:");
	// Log.log(w);
	// Log.log(" pW:");
	// Log.log(pW[side]);
	// Log.log(" pH:");
	// Log.log(pH[side]);
	// Log.log(" h:");
	// Log.log(h);

	int spriteX;
	int spriteY;
	int spriteW;
	int spriteH;

    if(pSource[side] != selected[side] || drawWhole[side]) {
        spriteX = gen[WIDTH]->get<int>()/2 + (side ? (gen[NEEDLE_CENTER_OFFSET]->get<int>() -gen[NEEDLE_CENTER_RADIUS]->get<int>()) : (-gen[ELLIPSE_A]->get<int>() + 1));
        spriteY = gen[HEIGHT]->get<int>()/2 - gen[ELLIPSE_B]->get<int>();
        spriteW = gen[ELLIPSE_A]->get<int>() - gen[NEEDLE_CENTER_OFFSET]->get<int>() + gen[NEEDLE_CENTER_RADIUS]->get<int>();
        spriteH = gen[ELLIPSE_B]->get<int>()*2;
    } else {
        spriteW = max(w, pW[side]);
        spriteX = side ? x1 : (gen[WIDTH]->get<int>()/2 - abs(gen[WIDTH]->get<int>()/2 - x1) - spriteW);
        spriteY = min(y1, pY1[side]);
        spriteH = pY1[side]+pH[side] > y1+h ? pY1[side]+pH[side]-spriteY : h+abs(y1-pY1[side]);
    }

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf("first calc: %lu", millis()-t2);
    t2 = millis();
#endif

    needleUpdate->setColorDepth(8);
    needleUpdate->createSprite(spriteW, spriteH);

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf(" draw scale: {", millis()-t2);
#endif

    drawScale(needleUpdate, side, spriteX, spriteY, 0, start, end);

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf("} total: %lu", millis()-t2);
    t2 = millis();
#endif

    int needleX = side ? gen[NEEDLE_CENTER_RADIUS]->get<int>() : (spriteW-gen[NEEDLE_CENTER_RADIUS]->get<int>());
    int needleY = deg >= 0 ? y1-spriteY+gen[NEEDLE_CENTER_RADIUS]->get<int>() : gen[HEIGHT]->get<int>()/2 - spriteY;

    needleUpdate->drawWedgeLine(
            needleX,
            needleY,
            side ? calcX(needleX, deg, length) : (spriteW-calcX(gen[NEEDLE_CENTER_RADIUS]->get<int>(), deg, length)),
            calcY(needleY, deg, length),
            gen[NEEDLE_BOTTOM_WIDTH]->get<int>(),
            gen[NEEDLE_TOP_WIDTH]->get<int>(),
            gen[NEEDLE_COLOR]->get<int>()
            );

//    needleUpdate->drawRect(
//        0,
//        0,
//        spriteW,
//        spriteH,
//        TFT_VIOLET);

	needleUpdate->fillCircle(
	        needleX,
	        needleY,
	        gen[NEEDLE_CENTER_RADIUS]->get<int>(),
	        gen[NEEDLE_CENTER_COLOR]->get<int>());

#ifdef LOG_DETAILED_FRAMETIME
	Log.logf(" draw needle: %lu", millis()-t2);
	t2 = millis();
#endif

	std::stringstream ss;
	ss.precision(settings->general[DATA_0 + selected[side] * DATA_SETTINGS_SIZE + DATA_PRECISION_OFFSET]->get<int>());
	ss << std::fixed << value;
	needleUpdate->setTextDatum(CC_DATUM);
	needleUpdate->setTextColor(gen[FONT_COLOR]->get<int>());
	needleUpdate->drawString(
	        ss.str().c_str(),
	        side ? gen[NEEDLE_CENTER_RADIUS]->get<int>() : spriteW-gen[NEEDLE_CENTER_RADIUS]->get<int>(),
	        gen[HEIGHT]->get<int>()/2 - spriteY);

#ifdef LOG_DETAILED_FRAMETIME
	Log.logf(" draw value: %lu", millis()-t2);
	t2 = millis();
#endif

	needleUpdate->pushSprite(spriteX + gen[OFFSET_X]->get<int>(), spriteY + gen[OFFSET_Y]->get<int>());
	needleUpdate->deleteSprite();

#ifdef LOG_DETAILED_FRAMETIME
	Log.logf(" push sprite: %lu ", millis()-t2);
	t2 = millis();
#endif

	pX1[side] = x1;
	pY1[side] = y1;
	pW[side] = w;
	pH[side] = h;
	pDeg[side] = deg;
	pSource[side] = selected[side];
    drawWhole[side] = false;
	xx+=2;
	if(xx>=360) {
	    xx-=360;
	}
}

int pMinute = -1, pDay = -1;

ulong t5;
void Screen::updateText(boolean force, int fps) {
    t5 = millis();
	DateTime now = data->getTime();

	if(	(now.month() > 3 && now.month() < 10) ||
		(now.month() == 3 && now.day() > 28))

	tft->setTextColor(gen[FONT_COLOR]->get<int>(), TFT_RED);
	tft->setAttribute(SFBG_ENABLE, true);
	tft->setTextDatum(CC_DATUM);
//	tft->setTextPadding(24);

	if(now.minute() != pMinute || force) {
		int min = now.minute();
//		tft->loadFont("GaugeHeavy"+(String)vis->timeSize);
        tft->loadFont("GaugeHeavyTime36", true);
		tft->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
		tft->setAttribute(SFBG_ENABLE, true);
		tft->setTextDatum(CC_DATUM);
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(2) << ((String)now.hour()).c_str() << ":" << std::setw(2) << ((String)min).c_str();
		tft->drawString(
			ss.str().c_str(),
			gen[WIDTH]->get<int>()/2+gen[OFFSET_X]->get<int>(),
			gen[HEIGHT]->get<int>()/2+gen[OFFSET_Y]->get<int>()+gen[TIME_POS_Y]->get<int>());
		pMinute = min;
	}
	if(now.day() != pDay || force) {
		std::stringstream ss2;
		ss2 << std::setfill('0') << std::setw(2) << ((String)now.day()).c_str() << "." << std::setw(2) << ((String)now.month()).c_str()  << "." << std::setw(2) << ((String)now.year()).substring(2).c_str();
//		tft->loadFont("GaugeHeavy"+(String)vis->dateSize);
		tft->loadFont("GaugeHeavy16");
		tft->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
		tft->setAttribute(SFBG_ENABLE, true);
		tft->setTextDatum(CC_DATUM);
		tft->drawString(
			ss2.str().c_str(),
			gen[WIDTH]->get<int>()/2+gen[OFFSET_X]->get<int>(),
			gen[HEIGHT]->get<int>()/2+gen[OFFSET_Y]->get<int>()+gen[DATE_POS_Y]->get<int>());
		pDay = now.day();
	}

#ifdef LOG_DETAILED_FRAMETIME
	Log.logf("time and date: %lu", millis()-t5);
	t5 = millis();
#endif

	std::stringstream ss3;
	ss3.precision(settings->general[DATA_0 + selected[MID] * DATA_SETTINGS_SIZE + DATA_PRECISION_OFFSET]->get<int>());
	ss3 << std::fixed << settings->general[DATA_0 + selected[MID] * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->get<float>() << settings->general[DATA_0 + selected[MID] * DATA_SETTINGS_SIZE + DATA_UNIT_OFFSET]->getString();
    const char* str = ss3.str().c_str();

	int w = textUpdate->textWidth(str);
	int h = textUpdate->fontHeight();
	textUpdate->setColorDepth(8);
	textUpdate->createSprite((gen[NEEDLE_CENTER_OFFSET]->get<int>()-gen[NEEDLE_CENTER_RADIUS]->get<int>())*2, h + SCALE_SPRITE_Y_OFFSET_12, 1);
	textUpdate->setTextColor(gen[FONT_COLOR]->get<int>());
	textUpdate->setTextDatum(TC_DATUM);
	textUpdate->drawString(str, textUpdate->width()/2, SCALE_SPRITE_Y_OFFSET_16);

	textUpdate->pushSprite(
	        gen[WIDTH]->get<int>()/2+gen[OFFSET_X]->get<int>() - textUpdate->width()/2,
	        gen[HEIGHT]->get<int>()/2+gen[OFFSET_Y]->get<int>()+80 - (textUpdate->height()/2 + SCALE_SPRITE_Y_OFFSET_16) / 2);
	textUpdate->deleteSprite();

#ifdef LOG_DETAILED_FRAMETIME
	Log.logf("data: %lu", millis()-t5);
	t5 = millis();
#endif
}

void Screen::showPrompt(String text, int lineSpacing, boolean useDefaultFont) {
    lock();
    switchView(PROMPT);

    if(!useDefaultFont)
        tft->loadFont("GaugeHeavy12");

    std::string str = text.c_str();
//    Log.log(str.c_str());

    std::size_t nextLine = 0;
    lines = 0;

    while(nextLine != std::string::npos) {

//        Log.log(str.c_str());
//        Log.log(nextLine);

        nextLine = str.find_first_of('\n');
        tft->drawString(str.substr(0, nextLine).c_str(), gen[WIDTH]->get<int>()/2, gen[HEIGHT]->get<int>()/2-40+(lines++)*(tft->fontHeight()+lineSpacing));
        str = str.substr(nextLine+1);
    }
    release();
}

void Screen::appendToPrompt(String text, int lineSpacing, boolean useDefaultFont) {
    lock();
    if(currentView != PROMPT) {
        Log.log("appendToPrompt() called without showPrompt()");
        return;
    }
    std::string str = text.c_str();
    std::size_t nextLine = 0;
    while(nextLine != std::string::npos) {
        nextLine = str.find_first_of('\n');
        tft->drawString(str.substr(0, nextLine).c_str(), gen[WIDTH]->get<int>()/2, gen[HEIGHT]->get<int>()/2-40+(lines++)*(tft->fontHeight()+lineSpacing));
        str = str.substr(nextLine+1);
    }
    release();
}

void Screen::drawSelectedInfo() {

    std::stringstream ss;
    ss << "<- " << std::nouppercase << settings->general[DATA_0 + selected[LEFT] * DATA_SETTINGS_SIZE + DATA_NAME_OFFSET]->getString() << "\n";
    ss << std::nouppercase << settings->general[DATA_0 + selected[RIGHT] * DATA_SETTINGS_SIZE + DATA_NAME_OFFSET]->getString() << " ->\n\n\n";
    ss << std::nouppercase << settings->general[DATA_0 + selected[MID] * DATA_SETTINGS_SIZE + DATA_NAME_OFFSET]->getString() << ":";
    std::string str = ss.str();
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c){ return std::tolower(c); });

    textUpdate->setColorDepth(8);
    textUpdate->createSprite(selectedInfoCoords[2], selectedInfoCoords[3], 1);
    textUpdate->setTextColor(gen[FONT_COLOR]->get<int>());
    textUpdate->setTextDatum(TC_DATUM);
    std::size_t nextLine = 0;
    int x = 0;

    while(nextLine != std::string::npos) {
        nextLine = str.find_first_of('\n');
        textUpdate->drawString(str.substr(0, nextLine).c_str(), textUpdate->width()/2, (x++)*(textUpdate->fontHeight()));
        str = str.substr(nextLine+1);
    }

    textUpdate->pushSprite(
            selectedInfoCoords[0],
            selectedInfoCoords[1]);
    textUpdate->deleteSprite();

    selectedInfoTimestamp = millis();
    selectedInfoVisible = true;
}

void Screen::clearSelectedInfo() {
    tft->fillRect(selectedInfoCoords[0],
                  selectedInfoCoords[1],
                  selectedInfoCoords[2],
                  selectedInfoCoords[3],
                  gen[BACKGROUND_COLOR]->get<int>());
    selectedInfoVisible = false;
}
