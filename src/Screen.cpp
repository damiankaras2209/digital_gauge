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
	Settings* settings = Settings::getInstance();
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

void Screen::init(TFT_eSPI *t, Data *d, Settings::DataSource *s) {
	tft = t;
	data = d;
	selected = s;
}

void Screen::reset() {
    lock();
    tft->fillScreen(settings->visual.backgroundColor);
	fillTables();
	drawScale(tft, false, 0, 0, 0, 0, settings->dataDisplay[selected[0]].scaleStart, settings->dataDisplay[selected[0]].scaleEnd);	//left
	drawScale(tft, false, 1, 0, 0, 0, settings->dataDisplay[selected[1]].scaleStart, settings->dataDisplay[selected[1]].scaleEnd);	//right
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
    switch(view) {
        case GAUGES:  {
            tft->fillScreen(settings->visual.backgroundColor);
            drawScale(tft, false, 0, 0, 0, 0, settings->dataDisplay[selected[0]].scaleStart, settings->dataDisplay[selected[0]].scaleEnd);	//left
            drawScale(tft, false, 1, 0, 0, 0, settings->dataDisplay[selected[1]].scaleStart, settings->dataDisplay[selected[1]].scaleEnd);	//right
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
    Serial.printf("Current view: %d", currentView);
}

unsigned long t;
void Screen::tick() {
    lock();
    switch(currentView) {
        case GAUGES:  {
            t = millis();
            Screen::getInstance()->updateText(false, 0);
            Screen::getInstance()->updateNeedle(0, selected[LEFT]);
            Screen::getInstance()->updateNeedle(1, selected[RIGHT]);
//            Serial.printf("Frametime: %lu\n", millis()-t);
            break;
        }
        case CLOCK:  {
            break;
        }
        case PROMPT:  {
            break;
        }
    }
    release();
    delay(1);
}

void Screen::drawScalePiece(void* c, boolean isSprite, int a, int b, int offset, int deg, int side, int originX, int originY, int length, int width, uint16_t color) {

    if(!side)
        side = -1;

    int m = 1;
    if(deg < 90) {
        deg = 90-deg;
    } else {
        deg-=90;
        m = -1;
    }

    //Serial.println(deg);

    double x1 = arrX[deg];
    double y1 = arrY[deg];
    double x2 = sqrt((arrR[deg]-length)*(arrR[deg]-length)/(1+tan(rad(deg))*tan(rad(deg))));
    double y2 = tan(rad(deg))*x2;

    //if(deg <1) {

    //tft->drawPixel(vis->width/2+vis->needleCenterOffset+x1,vis->height/2+y1, TFT_PINK);
    //tft->drawPixel(vis->width/2+vis->needleCenterOffset+x2,vis->height/2+y2, TFT_BLUE);


    // Serial.print(x1);
    // Serial.print(" ");
    // Serial.print(y1);
    // Serial.print(" ");
    // Serial.print(x2);
    // Serial.print(" ");
    // Serial.println(y2);

    (isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->drawWideLine(
            (isSprite ? 0 : vis->offsetX) + vis->width/2+side*(vis->needleCenterOffset+x1)-originX,
            (isSprite ? 0 : vis->offsetY) + vis->height/2+m*(y1)-originY,
            (isSprite ? 0 : vis->offsetX) + vis->width/2+side*(vis->needleCenterOffset+x2)-originX,
            (isSprite ? 0 : vis->offsetY) + vis->height/2+m*(y2)-originY,
            width,
            color,
            vis->backgroundColor);
    // }
}

unsigned long t3;

void Screen::drawScale(void* c, boolean isSprite, int side, int x1, int y1, int w, int start, int end) {

    t3 = millis();

    float stepSmall = 180/((vis->scaleLargeSteps*vis->scaleSmallSteps)*1.0);
    int steps = vis->scaleLargeSteps*vis->scaleSmallSteps+1;
    for(uint8_t i=0; i<steps; i++) {
        int16_t deg = stepSmall*i;
        drawScalePiece((isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c), isSprite, vis->ellipseA, vis->ellipseB, vis->needleCenterOffset, deg, side, x1, y1,
                       (i%(steps/vis->scaleLargeSteps)==0) ? vis->scaleLargeLength : vis->scaleSmallLength,
                       (i%(steps/vis->scaleLargeSteps)==0) ? vis->scaleLargeWidth : vis->scaleSmallWidth,
                       (i%vis->scaleAccColorEvery==0) ? vis->scaleAccColor : vis->scaleColor);
    }

//    Serial.printf(" draw scale pieces: %lu", millis()-t3);
//    t3 = millis();

    (isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->setTextColor(vis->fontColor);
    //	(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->setAttribute(SFBG_ENABLE, true);
    //	(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->setTextDatum(side ? CL_DATUM : CR_DATUM);
    //	(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->setTextDatum(CC_DATUM);
    (isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->setTextPadding(20);
    //	(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->loadFont("GaugeHeavyNumbers"+(String)vis->scaleSize);
    (isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->loadFont("GaugeHeavyNumbers12");

//    Serial.printf(" load font: %lu", millis()-t3);
//    t3 = millis();

    for(int i=0; i<5; i++) {
        int deg;

        switch(i) {
            case 0: deg = 90;break;
            case 1: deg = 45;break;
            case 2: deg = 0;break;
            case 3: deg = 45;break;
            case 4: deg = 90;break;
        }

        if(deg==0)
            (isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->setTextDatum(side ? CR_DATUM : CL_DATUM);
        else
            (isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->setTextDatum(CC_DATUM);


        int top = i>2 ? -1 : 1;

        if(!side)
            side = -1;

        (isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->drawString(
                ((String)(start + i*(end-start)/vis->scaleTextSteps)).c_str(),
                (isSprite ? 0 : vis->offsetX) + vis->width/2 + side*(vis->needleCenterOffset + calcX(0, deg, arrR[deg] - vis->scaleTextOffset))-x1,
                (isSprite ? 0 : vis->offsetY) + vis->height/2+ top*(calcY(0, deg, arrR[deg] - vis->scaleTextOffset))-y1
                );

    }

//    Serial.printf(" draw numbers: %lu", millis()-t3);
//    t3 = millis();
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
boolean firstRun[2] = {true, true};

unsigned long t2;

void Screen::updateNeedle(int side, Settings::DataSource source) {

    t2 = millis();

    int start, end;
    float value;

    value = settings->dataDisplay[source].value;
    start = settings->dataDisplay[source].scaleStart;
    end = settings->dataDisplay[source].scaleEnd;

//    value = (sin(xx/PI/18)/2+0.5)*(end-start)+start;

    float val = (value-start)/(end-start);

	if(val<0.0)
	    val = 0.0;
	if(val>1.0)
	    val = 1.0;

	double deg = (1.0-val)*180;
	// Serial.print(pos);
	// Serial.print(" ");
	// Serial.print(deg);
	deg-=90.0;
	// Serial.print(" ");
	// Serial.println(deg);

	//Serial.println(deg);
	// Serial.print(" ");
	// Serial.println(length);

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

	// Serial.print("x1:");
	// Serial.print(x1);
	// Serial.print(" pX1:");
	// Serial.print(pX1[side]);
	// Serial.print(" y1:");
	// Serial.print(y1);
	// Serial.print(" pY1:");
	// Serial.print(pY1[side]);
	// Serial.print(" w:");
	// Serial.print(w);
	// Serial.print(" pW:");
	// Serial.print(pW[side]);
	// Serial.print(" pH:");
	// Serial.print(pH[side]);
	// Serial.print(" h:");
	// Serial.println(h);



	TFT_eSprite update = TFT_eSprite(tft);
	
	update.setColorDepth(8);
	// update.setPaletteColor(pallete, 5);

	int spriteX;
	int spriteY;
	int spriteW;
	int spriteH;

    if(pSource[side] != source && !firstRun[side]) {
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

//    Serial.printf("first calc: %lu", millis()-t2);
//    t2 = millis();

    update.createSprite(spriteW, spriteH);

    drawScale(&update, true, side, spriteX, spriteY, 0, start, end);

//    Serial.printf(" draw scale: %lu", millis()-t2);
//    t2 = millis();

    int needleX = side ? vis->needleCenterRadius : (spriteW-vis->needleCenterRadius);
    int needleY = deg >= 0 ? y1-spriteY+vis->needleCenterRadius : vis->height/2 - spriteY;

    update.drawWedgeLine(
            needleX,
            needleY,
            side ? calcX(needleX, deg, length) : (spriteW-calcX(vis->needleCenterRadius, deg, length)),
            calcY(needleY, deg, length),
            vis->needleBottomWidth,
            vis->needleTopWidth,
            vis->needleColor
            );

//    update.drawRect(
//        0,
//        0,
//        spriteW,
//        spriteH,
//        TFT_VIOLET);

	update.fillCircle(
	        needleX,
	        needleY,
	        vis->needleCenterRadius,
	        vis->needleCenterColor);

//	Serial.printf(" draw needle: %lu", millis()-t2);
//	t2 = millis();

	std::stringstream ss;
	ss.precision(1);
	ss << std::fixed << value;
	update.setTextDatum(CC_DATUM);
	update.setTextColor(vis->fontColor);
	update.drawString(
	        ss.str().c_str(),
	        side ? vis->needleCenterRadius : spriteW-vis->needleCenterRadius,
	        vis->height/2 - spriteY);

//	Serial.printf(" draw value: %lu", millis()-t2);
//	t2 = millis();

	update.pushSprite(spriteX + vis->offsetX, spriteY + vis->offsetY);

//	Serial.printf(" push sprite: %lu\n", millis()-t2);
//	t2 = millis();

	pX1[side] = x1;
	pY1[side] = y1;
	pW[side] = w;
	pH[side] = h;
	pDeg[side] = deg;
	pSource[side] = source;
	firstRun[side] = false;
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
		tft->loadFont("GaugeHeavy36");
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
//    Serial.println(str.c_str());

    std::size_t nextLine = 0;
    lines = 0;

    while(nextLine != std::string::npos) {

//        Serial.println(str.c_str());
//        Serial.println(nextLine);

        nextLine = str.find_first_of('\n');
        tft->drawString(str.substr(0, nextLine).c_str(), vis->width/2, vis->height/2-40+(lines++)*(tft->fontHeight()+3+8));
        str = str.substr(nextLine+1);
    }
    release();
}

void Screen::appendToPrompt(String text) {
    lock();
    if(currentView != PROMPT) {
        Serial.print("appendToPrompt() called without showPrompt()");
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