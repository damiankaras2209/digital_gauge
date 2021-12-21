#include "Screen.h"

#define SCALE_SPRITE_Y_OFFSET 2

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
    vis = &(settings->visual);
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
	int a = vis->ellipseA;
	int b = vis->ellipseB;
	int offset = vis->needleCenterOffset;
	for(int i=0; i<91; i++) {
		double tga = tan(rad(i));
		double r = 0.0;

		if(i < 45) {
			for(int nx=1; nx<(a+1)*density; nx++) {
				double x = 1.0/density*nx;
				double y=tga*x;
				//tft->drawPixel(vis->width/2+vis->needleCenterOffset+x,vis->height/2+y, TFT_RED);
				if(1.0*(x+offset)*(x+offset)/(a*a)+1.0*y*y/(b*b)> 0.98) {
					//tft->drawPixel(vis->width/2+vis->needleCenterOffset+x,vis->height/2+y, TFT_WHITE);
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
				//tft->drawPixel(vis->width/2+vis->needleCenterOffset+x,vis->height/2+y, TFT_RED);
				if(1.0*(x+offset)*(x+offset)/(a*a)+1.0*y*y/(b*b)> 0.98) {
					//tft->drawPixel(vis->width/2+vis->needleCenterOffset+x,vis->height/2+y, TFT_WHITE);
					
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
            int start = settings->dataDisplay[selected[side]].scaleStart;
            int end = settings->dataDisplay[selected[side]].scaleEnd;

            String string = (String)(start + j*(end-start)/vis->scaleTextSteps);

            int w,h;
            scaleSprite[side][j]->deleteSprite();
            scaleSprite[side][j]->setColorDepth(8);
            scaleSprite[side][j]->loadFont("GaugeHeavyNumbers12");
            w = scaleSprite[side][j]->textWidth(string);
            h = scaleSprite[side][j]->fontHeight();
            scaleSprite[side][j]->createSprite(w, h + SCALE_SPRITE_Y_OFFSET, 1);
            scaleSprite[side][j]->setTextColor(settings->visual.fontColor);
            scaleSprite[side][j]->setTextDatum(TL_DATUM);
            scaleSprite[side][j]->drawString(string, 0, SCALE_SPRITE_Y_OFFSET);
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
        createScaleSprites(side);
        drawWhole[side] = true;
        release();
    }
}

void Screen::getSelected(Settings::DataSource* s) {
    for(int i=LEFT; i<SIDE_LAST; i++)
        s[i] = selected[i];
}

void Screen::reset() {
    lock();
    tft->fillScreen(settings->visual.backgroundColor);
    fillTables();
    createScaleSprites(LEFT);
    createScaleSprites(RIGHT);
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
            needleUpdate->loadFont("GaugeHeavyNumbers12");
            tft->fillScreen(settings->visual.backgroundColor);
            drawWhole[0] = true;
            drawWhole[1] = true;
            updateText(true, 0);
            break;
        }
        case CLOCK:  {
            break;
        }
        case PROMPT:  {
            tft->fillScreen(settings->visual.backgroundColor);
            tft->loadFont("GaugeHeavy12");
            tft->drawRect(vis->width/2-vis->promptWidth/2,
                          vis->height/2-vis->promptHeight/2,
                          vis->promptWidth,
                          vis->promptHeight,
                          vis->fontColor);
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
            Screen::getInstance()->updateText(false, 0);
            t1 = millis();
            Screen::getInstance()->updateNeedle(LEFT);
#ifdef LOG_DETAILED_FRAMETIME
            Log.logf(" total: %lu\n", millis()-t1);
            t1 = millis();
#endif
            Screen::getInstance()->updateNeedle(RIGHT);
#ifdef LOG_DETAILED_FRAMETIME
            Log.logf(" total: %lu ", millis()-t1);
#endif
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

    //tft->drawPixel(vis->width/2+vis->needleCenterOffset+x1,vis->height/2+y1, TFT_PINK);
    //tft->drawPixel(vis->width/2+vis->needleCenterOffset+x2,vis->height/2+y2, TFT_BLUE);


    // Log.log(x1);
    // Log.log(" ");
    // Log.log(y1);
    // Log.log(" ");
    // Log.log(x2);
    // Log.log(" ");
    // Log.log(y2);

    c->drawWideLine(
            vis->width/2 +side*(vis->needleCenterOffset+x1) - spriteX,
            vis->height/2 +m*(y1) - spriteY,
            vis->width/2 +side*(vis->needleCenterOffset+x2) - spriteX,
            vis->height/2 +m*(y2) - spriteY,
            width,
            color,
            vis->backgroundColor);
    // }
}

unsigned long t3;

void Screen::drawScale(TFT_eSprite* c, int side, int spriteX, int spriteY, int w, int start, int end) {

    t3 = millis();

    float stepSmall = 180/((vis->scaleLargeSteps*vis->scaleSmallSteps)*1.0);
    int steps = vis->scaleLargeSteps*vis->scaleSmallSteps+1;
    for(uint8_t i=0; i<steps; i++) {
        int16_t deg = stepSmall*i;
        drawScalePiece(c, deg, side, spriteX, spriteY,
                       (i%(steps/vis->scaleLargeSteps)==0) ? vis->scaleLargeLength : vis->scaleSmallLength,
                       (i%(steps/vis->scaleLargeSteps)==0) ? vis->scaleLargeWidth : vis->scaleSmallWidth,
                       (i%vis->scaleAccColorEvery==0) ? vis->scaleAccColor : vis->scaleColor);
    }

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
               vis->width/2 + (side ? 1 : -1)*(vis->needleCenterOffset + calcX(0, deg, arrR[deg] - vis->scaleTextOffset)) - spriteX - scaleSprite[side][i]->width()/2,
               vis->height/2 + top*(calcY(0, deg, arrR[deg] - vis->scaleTextOffset)) - spriteY - scaleSprite[side][i]->height()/2
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

    value = settings->dataDisplay[selected[side]].value;
    start = settings->dataDisplay[selected[side]].scaleStart;
    end = settings->dataDisplay[selected[side]].scaleEnd;

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

	int off = 5+vis->needleTopWidth;

	length = arrR[lround(abs(deg))];

	if(deg >= 0) {
		x = length*cos(rad(deg));
		y = length*sin(rad(deg));
		y1 = vis->height/2-vis->needleCenterRadius;
		h = max(y+vis->needleCenterRadius, vis->needleCenterRadius*2)+off;
//		tft->drawRect(x1, y1, w, h, TFT_RED);
	} else {
		y = length*cos(rad(deg+90));
		x = length*sin(rad(deg+90));
		y1 = vis->height/2-max((int)vis->needleCenterRadius, y)-off;
		h = max(y+vis->needleCenterRadius, vis->needleCenterRadius*2)+off;
//		tft->drawRect(x1, y1, w, h, TFT_RED);
	}

	x1 = vis->width/2+(vis->needleCenterOffset-vis->needleCenterRadius);
	w = max(x+vis->needleCenterRadius, vis->needleCenterRadius*2)+off;
	w = vis->ellipseA-vis->needleCenterOffset+vis->needleCenterRadius;

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
        spriteX = vis->width/2 + (side ? (vis->needleCenterOffset -vis->needleCenterRadius) : (-vis->ellipseA + 1));
        spriteY = vis->height/2 - vis->ellipseB;
        spriteW = vis->ellipseA - vis->needleCenterOffset + vis->needleCenterRadius;
        spriteH = vis->ellipseB*2;
    } else {
        spriteW = max(w, pW[side]);
        spriteX = side ? x1 : (vis->width/2 - abs(vis->width/2 - x1) - spriteW);
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

    int needleX = side ? vis->needleCenterRadius : (spriteW-vis->needleCenterRadius);
    int needleY = deg >= 0 ? y1-spriteY+vis->needleCenterRadius : vis->height/2 - spriteY;

    needleUpdate->drawWedgeLine(
            needleX,
            needleY,
            side ? calcX(needleX, deg, length) : (spriteW-calcX(vis->needleCenterRadius, deg, length)),
            calcY(needleY, deg, length),
            vis->needleBottomWidth,
            vis->needleTopWidth,
            vis->needleColor
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
	        vis->needleCenterRadius,
	        vis->needleCenterColor);

#ifdef LOG_DETAILED_FRAMETIME
	Log.logf(" draw needle: %lu", millis()-t2);
	t2 = millis();
#endif

	std::stringstream ss;
	ss.precision(1);
	ss << std::fixed << value;
	needleUpdate->setTextDatum(CC_DATUM);
	needleUpdate->setTextColor(vis->fontColor);
	needleUpdate->drawString(
	        ss.str().c_str(),
	        side ? vis->needleCenterRadius : spriteW-vis->needleCenterRadius,
	        vis->height/2 - spriteY);

#ifdef LOG_DETAILED_FRAMETIME
	Log.logf(" draw value: %lu", millis()-t2);
	t2 = millis();
#endif

	needleUpdate->pushSprite(spriteX + vis->offsetX, spriteY + vis->offsetY);
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

void Screen::updateText(boolean force, int fps) {
	DateTime now = data->getTime();

	if(	now.month() > 3 && now.month() < 10 ||
		now.month() == 3 && now.day() > 28)

	tft->setTextColor(vis->fontColor, TFT_RED);
	tft->setAttribute(SFBG_ENABLE, true);
	tft->setTextDatum(CC_DATUM);
//	tft->setTextPadding(24);

	if(now.minute() != pMinute || force) {
		int min = now.minute();
//		tft->loadFont("GaugeHeavy"+(String)vis->timeSize);
		tft->loadFont("GaugeHeavy36", true);
		tft->setTextColor(vis->fontColor, vis->backgroundColor);
		tft->setAttribute(SFBG_ENABLE, true);
		tft->setTextDatum(CC_DATUM);
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(2) << ((String)now.hour()).c_str() << ":" << std::setw(2) << ((String)min).c_str();
		tft->drawString(
			ss.str().c_str(),
			vis->width/2+vis->offsetX,
			vis->height/2+vis->offsetY+vis->timePosY);
		pMinute = min;
	}
	if(now.day() != pDay || force) {
		std::stringstream ss2;
		ss2 << std::setfill('0') << std::setw(2) << ((String)now.day()).c_str() << "." << std::setw(2) << ((String)now.month()).c_str()  << "." << std::setw(2) << ((String)now.year()).substring(2).c_str();
//		tft->loadFont("GaugeHeavy"+(String)vis->dateSize);
		tft->loadFont("GaugeHeavy16");
		tft->setTextColor(vis->fontColor, vis->backgroundColor);
		tft->setAttribute(SFBG_ENABLE, true);
		tft->setTextDatum(CC_DATUM);
		tft->drawString(
			ss2.str().c_str(),
			vis->width/2+vis->offsetX,
			vis->height/2+vis->offsetY+vis->datePosY);
		pDay = now.day();
	}

//	std::stringstream ss3;
//	ss3 << "frametime: " << ((String)fps).c_str();
//	tft->loadFont("GaugeHeavy"+(String)vis->dateSize);
//	tft->drawString(
//	        ss3.str().c_str(),
//	        vis->width/2+vis->offsetX,
//	        vis->height/2+vis->offsetY-vis->datePosY);

//	 int16_t val_0 = ads->readADC(0);
//  	 int16_t val_1 = ads->readADC(1);
//  	 int16_t val_2 = ads->readADC(2);
//  	 int16_t val_3 = ads->readADC(3);

//	 float f = ads->toVoltage(1);
//
//	 std::stringstream ss3;
//	 ss3 << ((String)(val_0 * f)).c_str() << "\n" << ((String)(val_1 * f)).c_str() << "\n" << ((String)(val_2 * f)).c_str() << "\n" << ((String)(val_3 * f)).c_str();
//	 tft->drawString(
//	 ss3.str().c_str(),
//	 vis->width/2+vis->offsetX,
//	 vis->height/2+vis->offsetY+vis->timePosY);
}

void Screen::showPrompt(String text) {
    lock();
    switchView(PROMPT);

    std::string str = text.c_str();
//    Log.log(str.c_str());

    std::size_t nextLine = 0;
    lines = 0;

    while(nextLine != std::string::npos) {

//        Log.log(str.c_str());
//        Log.log(nextLine);

        nextLine = str.find_first_of('\n');
        tft->drawString(str.substr(0, nextLine).c_str(), vis->width/2, vis->height/2-40+(lines++)*(tft->fontHeight()+3+8));
        str = str.substr(nextLine+1);
    }
    release();
}

void Screen::appendToPrompt(String text) {
    lock();
    if(currentView != PROMPT) {
        Log.log("appendToPrompt() called without showPrompt()");
        return;
    }
    std::string str = text.c_str();
    std::size_t nextLine = 0;
    while(nextLine != std::string::npos) {
        nextLine = str.find_first_of('\n');
        tft->drawString(str.substr(0, nextLine).c_str(), vis->width/2, vis->height/2-40+(lines++)*(tft->fontHeight()+3));
        str = str.substr(nextLine+1);
    }
    release();
}