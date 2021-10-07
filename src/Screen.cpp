#include "Screen.h"

Screen* Screen::screen = nullptr;

Screen::Screen() {
    isBusy = false;
    settings = Settings::getInstance();
}

Screen* Screen::getInstance()
{
    if(screen==nullptr)
        screen = new Screen();
    return screen;
}

uint16_t Screen::c24to16(int i) {
	return tft->color24to16(i);
}

double rad(int16_t deg) {
	return 1.0*deg * PI / 180;
}

float calcX(int16_t startX, int16_t deg, int16_t radius) {
	return startX + radius * cos(rad(deg));
}

float calcY(int16_t startY, int16_t deg, int16_t radius) {
	return startY + radius * sin(rad(deg));
}

void Screen::fillTables() {
	Settings* settings = Settings::getInstance();
	int density = 1;
	int a = settings->ellipseA;
	int b = settings->ellipseB;
	int offset = settings->needleCenterOffset;
	for(int i=0; i<91; i++) {
		double tga = tan(rad(i));
		double r = 0.0;

		if(i < 45) {
			for(int nx=1; nx<(a+1)*density; nx++) {
				double x = 1.0/density*nx;
				double y=tga*x;
				//tft->drawPixel(settings->width/2+settings->needleCenterOffset+x,settings->height/2+y, TFT_RED);
				if(1.0*(x+offset)*(x+offset)/(a*a)+1.0*y*y/(b*b)> 0.98) {
					//tft->drawPixel(settings->width/2+settings->needleCenterOffset+x,settings->height/2+y, TFT_WHITE);
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
				//tft->drawPixel(settings->width/2+settings->needleCenterOffset+x,settings->height/2+y, TFT_RED);
				if(1.0*(x+offset)*(x+offset)/(a*a)+1.0*y*y/(b*b)> 0.98) {
					//tft->drawPixel(settings->width/2+settings->needleCenterOffset+x,settings->height/2+y, TFT_WHITE);
					
					arrX[i] = x;
					arrY[i] = y;
					arrR[i] = sqrt(x*x+y*y);
					break;
				}
			}
		}
	}
}


unsigned long t2;

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

		//tft->drawPixel(settings->width/2+settings->needleCenterOffset+x1,settings->height/2+y1, TFT_PINK);
		//tft->drawPixel(settings->width/2+settings->needleCenterOffset+x2,settings->height/2+y2, TFT_BLUE);


		// Serial.print(x1);
		// Serial.print(" ");
		// Serial.print(y1);
		// Serial.print(" ");
		// Serial.print(x2);
		// Serial.print(" ");
		// Serial.println(y2);

	(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->drawWideLine(
        (isSprite ? 0 : settings->offsetX) + settings->width/2+side*(settings->needleCenterOffset+x1)-originX,
        (isSprite ? 0 : settings->offsetY) + settings->height/2+m*(y1)-originY,
        (isSprite ? 0 : settings->offsetX) + settings->width/2+side*(settings->needleCenterOffset+x2)-originX,
        (isSprite ? 0 : settings->offsetY) + settings->height/2+m*(y2)-originY,
		width, 
		color,
		settings->backgroundColor);
	// }
}



void Screen::drawScale(void* c, boolean isSprite, int side, int x1, int y1, int w) {
	int16_t start = 0, end = 180;

//	t2=millis();

	float stepSmall = (end-start)/((settings->scaleLargeSteps*settings->scaleSmallSteps)*1.0);
	int steps = settings->scaleLargeSteps*settings->scaleSmallSteps+1;
	for(uint8_t i=0; i<steps; i++) {
		int16_t deg = start+stepSmall*i;
		drawScalePiece((isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c), isSprite, settings->ellipseA, settings->ellipseB, settings->needleCenterOffset, deg, side, x1, y1,
		(i%(steps/settings->scaleLargeSteps)==0) ? settings->scaleLargeLength : settings->scaleSmallLength, 
		(i%(steps/settings->scaleLargeSteps)==0) ? settings->scaleLargeWidth : settings->scaleSmallWidth, 
		(i%settings->scaleAccColorEvery==0) ? settings->scaleAccColor : settings->scaleColor);

		// drawScalePiece((isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c), isSprite, settings->ellipseA, settings->ellipseB, settings->needleCenterOffset, deg, side, x1, y1,
		// settings->scaleSmallLength, 
		// settings->scaleSmallWidth, 
		// settings->scaleColor);

	}

//	Serial.print(" draw pieces: ");
//	Serial.print(millis()-t2);
//	t2=millis();


	// (isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->fillEllipse(
	// 	settings->width/2+settings->offsetX+x1,
	// 	settings->height/2+settings->offsetY+y1,
	// 	settings->ellipseA-settings->scaleLargeLength,
	// 	settings->ellipseB-settings->scaleLargeLength, 
	// 	settings->backgroundColor);
	// (isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->drawEllipse(
	// 	settings->width/2+settings->offsetX+x1,
	// 	settings->height/2+settings->offsetY+y1,
	// 	settings->ellipseA,
	// 	settings->ellipseB, 
	// 	settings->backgroundColor);
	// (isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->drawEllipse(
	// 	settings->width/2+settings->offsetX+x1,
	// 	settings->height/2+settings->offsetY+y1,
	// 	settings->ellipseA+1,
	// 	settings->ellipseB+1, 
	// 	settings->backgroundColor);

	(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->setTextColor(settings->fontColor, settings->backgroundColor);
	(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->setTextDatum(side ? CL_DATUM : CR_DATUM);
	(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->setTextPadding(20);
//	(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->loadFont("GaugeHeavyNumbers"+(String)settings->scaleSize);
	(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->loadFont("GaugeHeavyNumbers12");


//	Serial.print(" loading font: ");
//	Serial.print(millis()-t2);
//	t2=millis();

	for(int i=0; i<5; i++) {
		// Serial.print("i: ");
		// Serial.print(i);
		// Serial.print(", ");
		// Serial.print(side ? settings->scaleXRight[i]-x1 : settings->scaleXLeft[i]-x1);
		// Serial.print(", ");
		// Serial.println(settings->scaleY[i]-y1);

		if(side) {
			// if(settings->width/2+settings->offsetX+settings->needleCenterOffset+settings->scaleXRight[i] < x1+w+5) {
				// Serial.print(i);
				(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->drawString(
					((String)settings->scaleRight[i]).c_str(),
					settings->width/2+(isSprite ? 0 : settings->offsetX)+settings->needleCenterOffset+settings->scaleXRight[i]-x1,
					settings->height/2+(isSprite ? 0 : settings->offsetY)+settings->scaleY[i]-y1
				);
			// }

		} else {
			(isSprite ? (TFT_eSprite*)c : (TFT_eSPI*)c)->drawString(
				((String)settings->scaleLeft[i]).c_str(),
				settings->width/2+(isSprite ? 0 : settings->offsetX)-settings->needleCenterOffset+settings->scaleXLeft[i]-x1,
				settings->height/2+(isSprite ? 0 : settings->offsetY)+settings->scaleY[i]-y1
			);
		}
	}

//	Serial.print(" draw numbers: ");
//	Serial.print(millis()-t2);
//	t2=millis();

	// Serial.print("\n");
}

void Screen::init(TFT_eSPI *t, Data *d) {
	tft = t;
	data = d;
}

void Screen::blank() {
    tft->fillScreen(TFT_BLACK);
}

void Screen::reset() {
    if(isBusy)
        return;
    isBusy = true;
	tft->fillScreen(TFT_BLACK);
	fillTables();
	drawScale(tft, false, 0, 0, 0, 0);	//left
	drawScale(tft, false, 1, 0, 0, 0);	//right
    updateText(true, 0);

//	tft->setTextColor(settings->fontColor, settings->backgroundColor);
//	tft->setTextDatum(CC_DATUM);
//	// tft->setTextPadding(20);
//	tft->loadFont("GaugeHeavyNumbers"+(String)settings->scaleSize);
//	for(int i=0; i<5; i++)
//		tft->drawString(
//			((String)settings->scaleLeft[i]).c_str(),
//			settings->width/2+settings->offsetX-settings->needleCenterOffset+settings->scaleXLeft[i],
//			settings->height/2+settings->offsetY+settings->scaleY[i]
//		);
//	tft->setTextDatum(CC_DATUM);
//	for(int i=0; i<5; i++)
//		tft->drawString(
//			((String)settings->scaleRight[i]).c_str(),
//			settings->width/2+settings->offsetX+settings->needleCenterOffset+settings->scaleXRight[i],
//			settings->height/2+settings->offsetY+settings->scaleY[i]
//		);

	// update(c, rtc, 0.3);
	// update(c, rtc, 0.22);
	// update(c, rtc, 0.87);
	// update(c, rtc, 0.47);
	// update(c, rtc, 0.9);
	shallWeReset = false;
	isBusy = false;
}


int pX1[2], pY1[2], pW[2], pH[2], pDeg[2];

static const uint16_t pallete[] = {
	TFT_BLACK,		//  0  ^
 	TFT_GREEN,   	//  1  |
  	TFT_RED,   		//  2  |
  	TFT_DARKGREY,   //  3  |
	TFT_BLUE		//	4  |
};


void Screen::updateNeedle(int side, float value) {
    if(isBusy)
        return;
    isBusy = true;

    t2 = millis();


	float val = side ? value/10 : (value-30)/120;
//    val = value;
	if(val<0.0)
	    val = 0.0;
	if(val>1.0)
	    val = 1.0;

	float deg = (1.0-val)*180;
	// Serial.print(pos);
	// Serial.print(" ");
	// Serial.print(deg);
	deg-=90.0;
	// Serial.print(" ");
	// Serial.println(deg);

	float length = settings->needleLength-16*sin(8.7-(90-deg)/22.65);
	//Serial.println(deg);
	// Serial.print(" ");
	// Serial.println(length);

	int x, y, x1=0, y1=200, w=0, h=0;

	int off = 5+settings->needleTopWidth;

	if(deg >= 0) {
		x = length*cos(rad(deg));
		y = length*sin(rad(deg));
		y1 = settings->height/2-settings->needleCenterRadius;
		h = max(y+settings->needleCenterRadius, settings->needleCenterRadius*2)+off;
//		tft->drawRect(x1, y1, w, h, TFT_RED);
	} else {
		y = length*cos(rad(deg+90));
		x = length*sin(rad(deg+90));
		y1 = settings->height/2-max((int)settings->needleCenterRadius, y)-off;
		h = max(y+settings->needleCenterRadius, settings->needleCenterRadius*2)+off;
//		tft->drawRect(x1, y1, w, h, TFT_RED);
	}

	x1 = settings->width/2+(settings->needleCenterOffset-settings->needleCenterRadius);
	w = max(x+settings->needleCenterRadius, settings->needleCenterRadius*2)+off;
	w = settings->ellipseA-settings->needleCenterOffset+settings->needleCenterRadius;

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
	update.createSprite(max(w, pW[side]), pY1[side]+pH[side] > y1+h ? pY1[side]+pH[side]-min(y1, pY1[side]) : h+abs(y1-pY1[side]));
	if(side) {
//	    Serial.print("First calcs: ");
//	    Serial.print(millis()-t2);
//	    t2=millis();

	    drawScale(&update, true, side, x1, min(y1, pY1[side]), max(w, pW[side]));

//	    Serial.print("draw scale: ");
//	    Serial.print(millis()-t2);
//	    t2=millis();
		if(deg >= 0) {
			update.drawWedgeLine(
				settings->needleCenterRadius,
				y1-min(y1, pY1[side])+settings->needleCenterRadius,
				calcX(settings->needleCenterRadius, deg, length),
				calcY(y1-min(y1, pY1[side])+settings->needleCenterRadius, deg, length),
				settings->needleBottomWidth,
				settings->needleTopWidth,
				settings->needleColor
			);	
		} else {
			update.drawWedgeLine(
				settings->needleCenterRadius,
				settings->height/2 - min(y1, pY1[side]),
				calcX(settings->needleCenterRadius, deg, length),
				calcY(settings->height/2 - min(y1, pY1[side]), deg, length),
				settings->needleBottomWidth,
				settings->needleTopWidth,
				settings->needleColor
			);
		}

//		Serial.print(" 2nd calcs: ");
//		Serial.print(millis()-t2);
//		t2=millis();

		update.fillCircle(settings->needleCenterRadius, settings->height/2 - min(y1, pY1[side]), settings->needleCenterRadius, settings->needleCenterColor);
		std::stringstream ss;
		ss.precision(1);
		ss << std::fixed << value;
		update.setTextDatum(CC_DATUM);
		update.setTextColor(settings->fontColor, TFT_TRANSPARENT);
		update.drawString(ss.str().c_str(), settings->needleCenterRadius, settings->height/2 - min(y1, pY1[side]));
		update.pushSprite(x1 + settings->offsetX, min(y1, pY1[side]) + settings->offsetY);


//		Serial.print(" push calcs: ");
//		Serial.println(millis()-t2);



		// tft->drawRect(
		// 	x1, 
		// 	min(y1, pY1[side]), 
		// 	max(w, pW[side]), 
		// 	pY1[side]+pH[side] > y1+h ? pY1[side]+pH[side]-min(y1, pY1[side]) : h+abs(y1-pY1[side]),
		// 	TFT_VIOLET);
	} else {
	    drawScale(&update, true, side, settings->width/2 - abs(settings->width/2 - x1) - max(w, pW[side]), min(y1, pY1[side]), max(w, pW[side]));
		if(deg >= 0) {
			update.drawWedgeLine(
            max(w, pW[side])-settings->needleCenterRadius,
            y1-min(y1, pY1[side])+settings->needleCenterRadius,
            max(w, pW[side])-calcX(settings->needleCenterRadius, deg, length),
            calcY(y1-min(y1, pY1[side])+settings->needleCenterRadius, deg, length),
                settings->needleBottomWidth,
				settings->needleTopWidth,
				settings->needleColor
			);	
		} else {
			update.drawWedgeLine(
				max(w, pW[side])-settings->needleCenterRadius,
				settings->height/2 - min(y1, pY1[side]),
				max(w, pW[side])-calcX(settings->needleCenterRadius, deg, length),
				calcY(settings->height/2 - min(y1, pY1[side]), deg, length),
				settings->needleBottomWidth,
				settings->needleTopWidth,
				settings->needleColor
			);
		}

		update.fillCircle(max(w, pW[side])-settings->needleCenterRadius, settings->height/2 - min(y1, pY1[side]), settings->needleCenterRadius, settings->needleCenterColor);
		std::stringstream ss;
		ss.precision(1);
		ss << std::fixed << value;
		update.setTextDatum(CC_DATUM);
		update.setTextColor(settings->fontColor, TFT_TRANSPARENT);
		update.drawString(ss.str().c_str(), max(w, pW[side])-settings->needleCenterRadius, settings->height/2 - min(y1, pY1[side]));
		update.pushSprite(settings->width/2 - abs(settings->width/2 - x1) - max(w, pW[side])  + settings->offsetX, min(y1, pY1[side]) + settings->offsetY);
		// tft->drawRect(
		// 	settings->width/2 - abs(settings->width/2 - x1) - max(w, pW[side]), 
		// 	min(y1, pY1[side]), 
		// 	max(w, pW[side]), 
		// 	pY1[side]+pH[side] > y1+h ? pY1[side]+pH[side]-min(y1, pY1[side]) : h+abs(y1-pY1[side]),
		// 	TFT_VIOLET);
	}
	
	pX1[side] = x1;
	pY1[side] = y1;
	pW[side] = w;
	pH[side] = h;
	pDeg[side] = deg;
	isBusy = false;
}

int pMinute = -1, pDay = -1;

void Screen::updateText(boolean force, int fps) {
    if(isBusy)
        return;
    isBusy = true;
	DateTime now = data->getTime();

	if(	now.month() > 3 && now.month() < 10 ||
		now.month() == 3 && now.day() > 28)

	tft->setTextColor(settings->fontColor, settings->backgroundColor);
	tft->setTextDatum(CC_DATUM);
	tft->setTextPadding(24);

	if(now.minute() != pMinute || force) {
		int min = now.minute();
//		tft->loadFont("GaugeHeavy"+(String)settings->timeSize);
		tft->loadFont("GaugeHeavy36");
		std::stringstream ss;
		ss << std::setfill('0') << std::setw(2) << ((String)now.hour()).c_str() << ":" << std::setw(2) << ((String)min).c_str();
		tft->drawString(
			ss.str().c_str(),
			settings->width/2+settings->offsetX,
			settings->height/2+settings->offsetY+settings->timePosY);
		pMinute = min;
	}
	if(now.day() != pDay || force) {
		std::stringstream ss2;
		ss2 << std::setfill('0') << std::setw(2) << ((String)now.day()).c_str() << "." << std::setw(2) << ((String)now.month()).c_str()  << "." << std::setw(2) << ((String)now.year()).substring(2).c_str();
//		tft->loadFont("GaugeHeavy"+(String)settings->dateSize);
		tft->loadFont("GaugeHeavy16");
		tft->drawString(
			ss2.str().c_str(),
			settings->width/2+settings->offsetX,
			settings->height/2+settings->offsetY+settings->datePosY);
		pDay = now.day();
	}

//	std::stringstream ss3;
//	ss3 << "frametime: " << ((String)fps).c_str();
//	tft->loadFont("GaugeHeavy"+(String)settings->dateSize);
//	tft->drawString(
//	        ss3.str().c_str(),
//	        settings->width/2+settings->offsetX,
//	        settings->height/2+settings->offsetY-settings->datePosY);

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
//	 settings->width/2+settings->offsetX,
//	 settings->height/2+settings->offsetY+settings->timePosY);
    isBusy = false;
}

void Screen::setPromptFont() {
    tft->loadFont("GaugeHeavy12");
}


void Screen::showPrompt(String text) {
    promptShown = true;
    setPromptFont();
    tft->fillScreen(TFT_BLACK);
    tft->drawRect(settings->width/2-settings->promptWidth/2,
                  settings->height/2-settings->promptHeight/2,
                  settings->promptWidth,
                  settings->promptHeight,
                  settings->fontColor);
    tft->setTextDatum(CC_DATUM);

    std::string str = text.c_str();

//    Serial.println(str.c_str());

    std::size_t nextLine = 0;
    lines = 0;

    while(nextLine != std::string::npos) {

//        Serial.println(str.c_str());
//        Serial.println(nextLine);

        nextLine = str.find_first_of('\n');
        tft->drawString(str.substr(0, nextLine).c_str(), settings->width/2, settings->height/2-40+(lines++)*(tft->fontHeight()+3+8));
        str = str.substr(nextLine+1);
    }
}

void Screen::addToPrompt(String text) {
    if(!promptShown) {
        Serial.print("addToPrompt() called without showPrompt()");
        return;
    }
    setPromptFont();
    std::string str = text.c_str();
    std::size_t nextLine = 0;
    while(nextLine != std::string::npos) {
        nextLine = str.find_first_of('\n');
        tft->drawString(str.substr(0, nextLine).c_str(), settings->width/2, settings->height/2-40+(lines++)*(tft->fontHeight()+3));
        str = str.substr(nextLine+1);
    }
}