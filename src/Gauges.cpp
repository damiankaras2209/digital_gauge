#include "Gauges.h"

#define TARGET (isSprite ? (TFT_eSprite*)target : (TFT_eSPI*)target)

double rad(double deg) {
    return 1.0 * deg * PI / 180;
}

double calcX(int16_t startX, double deg, int16_t radius) {
    return startX + radius * cos(rad(deg));
}

double calcY(int16_t startY, double deg, int16_t radius) {
    return startY + radius * sin(rad(deg));
}

void Gauges::init(TFT_eSPI *t, Lock *l) {
    tft = t;
    lock = l;
    gen = Settings.general;
    selected = Settings.state.selected;
    setSelectedFromState();
    for(int i=0; i<2; i++) {
        for(int j=0; j<5; j++) {
            scaleSprite[i][j] = new TFT_eSprite(tft);
        }
    }
    needleUpdate = new TFT_eSprite(tft);
    textUpdate = new TFT_eSprite(tft);

    for(int i=0; i<SIDE_SIZE; i++)
        clickables.push_back(new Clickable);

    for(bool & icon : icons)
        icon = false;

    reInit();

}

void Gauges::reInit() {
    fillTables();
    prepare();
    selectedInfoCoords[2] = (gen[NEEDLE_CENTER_OFFSET]->get<int>() - gen[NEEDLE_CENTER_RADIUS]->get<int>()) * 2; //width
    selectedInfoCoords[3] = (textUpdate->fontHeight()+GAUGES_LINE_SPACING)*4 - GAUGES_LINE_SPACING + SCALE_SPRITE_Y_OFFSET_16; //height
    selectedInfoCoords[0] = gen[WIDTH]->get<int>()/2 + gen[OFFSET_X]->get<int>() - selectedInfoCoords[2]/2; //x
    selectedInfoCoords[1] = gen[HEIGHT]->get<int>()/2 + gen[OFFSET_Y]->get<int>() + 5; //y
    for(auto & s : redraw)
        s = true;

    clickables.at(LEFT)->setPos(
            gen[WIDTH]->get<int>()/2 - gen[ELLIPSE_A]->get<int>() + gen[OFFSET_X]->get<int>(),
            gen[HEIGHT]->get<int>()/2 - gen[ELLIPSE_B]->get<int>() + gen[OFFSET_Y]->get<int>());
    clickables.at(LEFT)->setSize(
            gen[ELLIPSE_A]->get<int>() - gen[NEEDLE_CENTER_OFFSET]->get<int>(),
            gen[ELLIPSE_B]->get<int>()*2);

    clickables.at(RIGHT)->setPos(
            gen[WIDTH]->get<int>()/2 + gen[NEEDLE_CENTER_OFFSET]->get<int>() + gen[OFFSET_X]->get<int>(),
            gen[HEIGHT]->get<int>()/2 - gen[ELLIPSE_B]->get<int>() + gen[OFFSET_Y]->get<int>());
    clickables.at(RIGHT)->setSize(
            gen[ELLIPSE_A]->get<int>() - gen[NEEDLE_CENTER_OFFSET]->get<int>(),
            gen[ELLIPSE_B]->get<int>()*2);

    clickables.at(MID)->setPos(
            gen[WIDTH]->get<int>()/2 - gen[NEEDLE_CENTER_OFFSET]->get<int>() + gen[OFFSET_X]->get<int>(),
            gen[HEIGHT]->get<int>()/2 + gen[OFFSET_Y]->get<int>());
    clickables.at(MID)->setSize(
            gen[NEEDLE_CENTER_OFFSET]->get<int>()*2,
            gen[ELLIPSE_B]->get<int>());

    clickables.at(TIME)->setPos(
            gen[WIDTH]->get<int>()/2 - gen[NEEDLE_CENTER_OFFSET]->get<int>() + gen[OFFSET_X]->get<int>(),
            gen[HEIGHT]->get<int>()/2 - gen[ELLIPSE_B]->get<int>() + gen[OFFSET_Y]->get<int>());
    clickables.at(TIME)->setSize(
            gen[NEEDLE_CENTER_OFFSET]->get<int>()*2,
            gen[ELLIPSE_B]->get<int>());
}

void Gauges::prepare() {
    needleUpdate->loadFont("GaugeHeavyNumbers12");
    textUpdate->loadFont("GaugeHeavy16");
    createScaleSprites(LEFT);
    createScaleSprites(RIGHT);
}

void Gauges::clean() {
    needleUpdate->unloadFont();
    textUpdate->unloadFont();
    for(auto & i : scaleSprite)
        for(auto & j : i)
            j->deleteSprite();
}

void Gauges::setSelectedFromState() {
    for(int i=0; i<SettingsClass::DataSource::VOLTAGE+1; i++)
        Data.dataInput[i].visible = false;
    for(int i=LEFT; i<3; i++)
        Data.dataInput[selected[i]].visible = true;
}

void Gauges::setSelected(Side side, SettingsClass::DataSource s) {
    if(selected[side] != s) {
        lock->lock();
        selected[side] = s;

        for(int i=0; i<SettingsClass::DataSource::VOLTAGE+1; i++)
            Data.dataInput[i].visible = false;
        for(int i=LEFT; i<3; i++)
            Data.dataInput[selected[i]].visible = true;

        if(side != MID) {
            redraw[side] = true;
            createScaleSprites(side);
        }
        drawSelectedInfo();
        lock->release();
        Settings.saveState();
    }
}

void Gauges::getSelected(SettingsClass::DataSource* s) {
    for(int i=LEFT; i<SIDE_SIZE; i++)
        s[i] = selected[i];
}



void Gauges::cycleData(Side side) {
    if(side != SIDE_SIZE && side != TIME) {
        SettingsClass::DataSource s = selected[side];
        Log.logf_d("Current data: %s\n", Settings.dataSourceString[selected[side]].c_str());
        do {
            s = static_cast<SettingsClass::DataSource>(s + 1);
            if(s == SettingsClass::LAST)
                s = static_cast<SettingsClass::DataSource>(0);
        } while (!Settings.general[DATA_BEGIN_BEGIN + s * DATA_SETTINGS_SIZE + DATA_ENABLE_OFFSET]->get<bool>());
        Log.logf_d("Changing to: %s\n", Settings.dataSourceString[s].c_str());
        setSelected(side, s);
    }
}

void Gauges::fillTables() {
    int density = 1;
    int a = gen[ELLIPSE_A]->get<int>();
    int b = gen[ELLIPSE_B]->get<int>();
    int offset = gen[NEEDLE_CENTER_OFFSET]->get<int>();
    double rmin = 1000.0;

    for(int i=0; i<91; i++) {
        double tga = tan(rad(i));

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
        if(arrR[i] < rmin)
            rmin = arrR[i];
    }

    for(int i=0; i<91; i++) {
        arrOffset[i] = arrR[i] - rmin;
    }
}

//ulong t4;
void Gauges::createScaleSprites(Side side) {
    //    t4 = millis();
    if(side != MID) {
        for(int j=0; j<5; j++) {
            int start = gen[DATA_BEGIN_BEGIN + selected[side] * DATA_SETTINGS_SIZE + DATA_SCALE_START_OFFSET]->get<int>();
            int end = gen[DATA_BEGIN_BEGIN + selected[side] * DATA_SETTINGS_SIZE + DATA_SCALE_END_OFFSET]->get<int>();

            String string = (String)(start + j*(end-start)/gen[SCALE_TEXT_STEPS]->get<int>());

            int w,h;
            scaleSprite[side][j]->deleteSprite();
            scaleSprite[side][j]->setColorDepth(8);
            scaleSprite[side][j]->loadFont("GaugeHeavyNumbers12");
            w = scaleSprite[side][j]->textWidth(string);
            h = scaleSprite[side][j]->fontHeight();
            scaleSprite[side][j]->createSprite(w, h + SCALE_SPRITE_Y_OFFSET_12, 1);
            scaleSprite[side][j]->fillSprite(gen[BACKGROUND_COLOR]->get<int>());
            scaleSprite[side][j]->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
            scaleSprite[side][j]->setTextDatum(TL_DATUM);
            scaleSprite[side][j]->drawString(string, 0, SCALE_SPRITE_Y_OFFSET_12);
            scaleSprite[side][j]->unloadFont();
            delay(1);
        }
    }
    //    Log.logf("Sprites creation time: %lu", millis()-t4);
}

void Gauges::drawScalePiece(void* target, bool isSprite, int deg, int side, int offsetX, int offsetY, int length, int width, uint16_t color) {

    if(!side)
        side = -1;

    int m = 1;
    if(deg < 90) {
        deg = 90-deg;
    } else {
        deg-=90;
        m = -1;
    }

    //Log.logf(deg);

    double x1 = arrX[deg];
    double y1 = arrY[deg];
    double x2 = sqrt((arrR[deg]-length)*(arrR[deg]-length)/(1+tan(rad(deg))*tan(rad(deg))));
    double y2 = tan(rad(deg))*x2;

    //if(deg <1) {

    //tft->drawPixel(gen[WIDTH]->get<int>()/2+gen[NEEDLE_CENTER_OFFSET]->get<int>()+x1,gen[HEIGHT]->get<int>()/2+y1, TFT_PINK);
    //tft->drawPixel(gen[WIDTH]->get<int>()/2+gen[NEEDLE_CENTER_OFFSET]->get<int>()+x2,gen[HEIGHT]->get<int>()/2+y2, TFT_BLUE);


    // Log.logf(x1);
    // Log.logf(" ");
    // Log.logf(y1);
    // Log.logf(" ");
    // Log.logf(x2);
    // Log.logf(" ");
    // Log.logf(y2);

    TARGET->drawWideLine(
            gen[WIDTH]->get<int>()/2 +side*(gen[NEEDLE_CENTER_OFFSET]->get<int>()+x1) + offsetX,
            gen[HEIGHT]->get<int>()/2 +m*(y1) + offsetY,
            gen[WIDTH]->get<int>()/2 +side*(gen[NEEDLE_CENTER_OFFSET]->get<int>()+x2) + offsetX,
            gen[HEIGHT]->get<int>()/2 +m*(y2) + offsetY,
            width,
            color,
            gen[BACKGROUND_COLOR]->get<int>());
    // }
}

unsigned long t3;

void Gauges::drawScale(void *target, bool isSprite, int side, int offsetX, int offsetY, int w, int start, int end) {

    t3 = millis();

    float stepSmall = 180/((gen[SCALE_LARGE_STEPS]->get<int>()*gen[SCALE_SMALL_STEPS]->get<int>())*1.0);
    int steps = gen[SCALE_LARGE_STEPS]->get<int>()*gen[SCALE_SMALL_STEPS]->get<int>()+1;
    for(uint8_t i=0; i<steps; i++) {
        int16_t deg = stepSmall*i;
        drawScalePiece(target, isSprite, deg, side, offsetX, offsetY,
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

        int x = gen[WIDTH]->get<int>()/2 + (side ? 1 : -1)*(gen[NEEDLE_CENTER_OFFSET]->get<int>() + calcX(0, deg, arrR[deg] - gen[SCALE_TEXT_OFFSET]->get<int>())) - scaleSprite[side][i]->width()/2 + offsetX;
        int y = gen[HEIGHT]->get<int>()/2 + top*(calcY(0, deg, arrR[deg] - gen[SCALE_TEXT_OFFSET]->get<int>())) - scaleSprite[side][i]->height()/2 + offsetY;

        if(isSprite)
            scaleSprite[side][i]->pushToSprite(needleUpdate, x, y);
        else
            scaleSprite[side][i]->pushSprite(x, y);

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

int pX[2], pY[2], pW[2], pH[2], pDeg[2];
SettingsClass::DataSource pSource[2];

unsigned long t2;

void Gauges::updateNeedle(int side) {

    t2 = millis();

    int start, end;
    double value;

    if(gen[DEMO]->get<bool>()) {
        value =  (sin((xx + (side ? 0 : 180))/PI/18)/2+0.5);
        start = 0;
        end = 1;
    } else {
        value = Data.dataInput[selected[side]].value;
//        value = gen[DATA_BEGIN_BEGIN + selected[side] * DATA_SETTINGS_SIZE + DATA_VALUE_OFFSET]->get<float>();
        start = gen[DATA_BEGIN_BEGIN + selected[side] * DATA_SETTINGS_SIZE + DATA_SCALE_START_OFFSET]->get<int>();
        end = gen[DATA_BEGIN_BEGIN + selected[side] * DATA_SETTINGS_SIZE + DATA_SCALE_END_OFFSET]->get<int>();
    }

    double val = (value-start)/(end-start);

    if(val<0.0)
        val = 0.0;
    if(val>1.0)
        val = 1.0;

    auto deg = (1.0 - val)*180;
    deg -= 90.0;

    //calculate dimensions
    double needleLength;
    int needleBBoxW, needleBBoxH;
    int x, y, w, h;

    int off = gen[NEEDLE_TOP_WIDTH]->get<int>() + 1;

    needleLength = gen[NEEDLE_LENGTH]->get<double>();
    if(gen[NEEDLE_LENGTH_ADAPTIVE]->get<bool>()) {
        needleLength += arrOffset[lround(abs(deg))];
    }

    if(deg >= 0) {
        needleBBoxW = lround(needleLength * cos(rad(deg))) + off;
        needleBBoxH = lround(needleLength * sin(rad(deg))) + off;
        y = gen[HEIGHT]->get<int>() / 2 - gen[NEEDLE_CENTER_RADIUS]->get<int>();
    } else {
        needleBBoxW = lround(needleLength * sin(rad(deg + 90))) + off;
        needleBBoxH = lround(needleLength * cos(rad(deg + 90))) + off;
        y = gen[HEIGHT]->get<int>() / 2 - max(gen[NEEDLE_CENTER_RADIUS]->get<int>(), needleBBoxH);
    }

    x = gen[WIDTH]->get<int>() / 2 + (gen[NEEDLE_CENTER_OFFSET]->get<int>() - gen[NEEDLE_CENTER_RADIUS]->get<int>());
    w = gen[ELLIPSE_A]->get<int>()-gen[NEEDLE_CENTER_OFFSET]->get<int>()+gen[NEEDLE_CENTER_RADIUS]->get<int>();
    w = max(needleBBoxW + gen[NEEDLE_CENTER_RADIUS]->get<int>(), gen[NEEDLE_CENTER_RADIUS]->get<int>() * 2);
    h = max(needleBBoxH + gen[NEEDLE_CENTER_RADIUS]->get<int>(), gen[NEEDLE_CENTER_RADIUS]->get<int>() * 2);


    //enlarge the area with the previous dimensions
    int areaX;
    int areaY;
    int areaW;
    int areaH;

    if(pSource[side] != selected[side] || redraw[side]) {
        areaX = gen[WIDTH]->get<int>() / 2 + (side ? (gen[NEEDLE_CENTER_OFFSET]->get<int>() - gen[NEEDLE_CENTER_RADIUS]->get<int>()) : -gen[ELLIPSE_A]->get<int>());
        areaY = gen[HEIGHT]->get<int>() / 2 - gen[ELLIPSE_B]->get<int>();
        areaW = gen[ELLIPSE_A]->get<int>() - gen[NEEDLE_CENTER_OFFSET]->get<int>() + gen[NEEDLE_CENTER_RADIUS]->get<int>();
        areaH = gen[ELLIPSE_B]->get<int>() * 2;
    } else {
        areaW = max(w, pW[side]);
        areaX = side ? x : (gen[WIDTH]->get<int>() / 2 - abs(gen[WIDTH]->get<int>() / 2 - x) - areaW);
        areaY = min(y, pY[side]);
        areaH = max(pY[side] + pH[side], y + h) - areaY;;
    }


    //prepare sprite or tft
    void* target = needleUpdate;
    bool isSprite = true;
    int offsetX = 0;
    int offsetY = 0;

    needleUpdate->setColorDepth(8);
    if(!needleUpdate->createSprite(areaW, areaH)) {
        Log.logf_d("Unable to create needle sprite");
        isSprite = false;
        target = tft;
        tft->loadFont("GaugeHeavyNumbers12");
        offsetX = gen[OFFSET_X]->get<int>();
        offsetY = gen[OFFSET_Y]->get<int>();
    }

    if(isSprite)
        needleUpdate->fillSprite(gen[BACKGROUND_COLOR]->get<int>());
    else {
        tft->fillRect(areaX + offsetX, areaY + offsetY, areaW, areaH, gen[BACKGROUND_COLOR]->get<int>());
        areaY = 0;
        areaX = 0;
    }

    offsetX = - areaX + offsetX;
    offsetY = - areaY + offsetY;

//    needleUpdate->drawRect(x + offsetX, y + offsetY, w, h, TFT_RED);

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf("first calc: %lu draw scale: {", millis()-t2);
    t2 = millis();
#endif

    drawScale(target, isSprite, side, offsetX, offsetY, 0, start, end);

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf("} total: %lu", millis()-t2);
    t2 = millis();
#endif

    //absolute location of needle center on screen without offset
    int needleX = gen[WIDTH]->get<int>()/2 + (side ? 1 : -1)*gen[NEEDLE_CENTER_OFFSET]->get<int>();
    int needleY = gen[HEIGHT]->get<int>()/2;

    TARGET->drawWedgeLine(
        needleX + offsetX,
        needleY + offsetY,
        needleX + (side ? 1 : -1)*calcX(0, deg, needleLength) + offsetX,
        needleY + calcY(0, deg, needleLength) + offsetY,
        gen[NEEDLE_BOTTOM_WIDTH]->get<int>(),
        gen[NEEDLE_TOP_WIDTH]->get<int>(),
        gen[NEEDLE_COLOR]->get<int>()
    );

//    needleUpdate->drawRect(0, 0, areaW, areaH, TFT_VIOLET);

    TARGET->fillCircle(
        needleX + offsetX,
        needleY + offsetY,
        gen[NEEDLE_CENTER_RADIUS]->get<int>(),
        gen[NEEDLE_CENTER_COLOR]->get<int>()
    );

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf(" draw needle: %lu", millis()-t2);
    t2 = millis();
#endif

    std::stringstream ss;
    ss.precision(gen[DATA_BEGIN_BEGIN + selected[side] * DATA_SETTINGS_SIZE + DATA_PRECISION_OFFSET]->get<int>());
    if (value > 1000 || value < 0)
        ss << (int) value;
    else if(isnan(value))
        ss << "-";
    else
        ss << std::fixed << value;
    TARGET->setTextDatum(CC_DATUM);
    TARGET->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
    TARGET->drawString(
            ss.str().c_str(),
            gen[WIDTH]->get<int>()/2 + (side ? 1 : -1)*(gen[NEEDLE_CENTER_OFFSET]->get<int>()) + offsetX,
            gen[HEIGHT]->get<int>()/2 + offsetY
    );

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf(" draw value: %lu", millis()-t2);
    t2 = millis();
#endif

    if(isSprite) {
        needleUpdate->pushSprite(areaX + gen[OFFSET_X]->get<int>(), areaY + gen[OFFSET_Y]->get<int>());
        needleUpdate->deleteSprite();
    }

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf(" push sprite: %lu ", millis()-t2);
    t2 = millis();
#endif

    pX[side] = x;
    pY[side] = y;
    pW[side] = w;
    pH[side] = h;
    pDeg[side] = deg;
    pSource[side] = selected[side];
    redraw[side] = false;

    if(gen[DEMO]->get<bool>()) {
        xx+=2;
        if(xx>=360) {
            xx-=360;
        }
    }
}

int pMinute = -1, pDay = -1;
int pTimeWidth = 0, pDateWidth = 0;

ulong t5;
void Gauges::updateText() {
    t5 = millis();
    DateTime now = Data.getTime();

    if(	(now.month() > 3 && now.month() < 10) ||
    (now.month() == 3 && now.day() > 28))

    tft->setTextColor(gen[FONT_COLOR]->get<int>(), TFT_RED);
    tft->setTextDatum(CC_DATUM);
    //	tft->setTextPadding(24);

    if(now.minute() != pMinute || redraw[TIME]) {
        int min = now.minute();
        //		tft->loadFont("GaugeHeavy"+(String)vis->timeSize);
        tft->loadFont("GaugeHeavyTime36", true);
        tft->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
        tft->setTextDatum(CC_DATUM);
        std::stringstream ss;
        ss << std::setfill('0') << std::setw(2) << ((String)now.hour()).c_str() << ":" << std::setw(2) << ((String)min).c_str();
        int width = tft->textWidth(ss.str().c_str());
        int w = max(width, pTimeWidth);
        int h = tft->fontHeight() + 8;
        tft->fillRect(
                gen[WIDTH]->get<int>()/2+gen[OFFSET_X]->get<int>() - w/2,
                gen[HEIGHT]->get<int>()/2+gen[OFFSET_Y]->get<int>()+gen[TIME_POS_Y]->get<int>() - h/2 - 4,
                w,
                h,
                gen[BACKGROUND_COLOR]->get<int>()
                );
        tft->drawString(
                ss.str().c_str(),
                gen[WIDTH]->get<int>()/2+gen[OFFSET_X]->get<int>(),
                gen[HEIGHT]->get<int>()/2+gen[OFFSET_Y]->get<int>()+gen[TIME_POS_Y]->get<int>());
        pTimeWidth = width;
        pMinute = min;
    }
    if(now.day() != pDay || redraw[TIME]) {
        std::stringstream ss2;
        ss2 << std::setfill('0') << std::setw(2) << ((String)now.day()).c_str() << "." << std::setw(2) << ((String)now.month()).c_str()  << "." << std::setw(2) << ((String)now.year()).substring(2).c_str();
        //		tft->loadFont("GaugeHeavy"+(String)vis->dateSize);
        tft->loadFont("GaugeHeavy16");
        tft->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
        tft->setTextDatum(CC_DATUM);
        int width = tft->textWidth(ss2.str().c_str());
        int w = max(width, pDateWidth);
        int h = tft->fontHeight() + 2;
        tft->fillRect(
                gen[WIDTH]->get<int>()/2+gen[OFFSET_X]->get<int>() - w/2,
                gen[HEIGHT]->get<int>()/2+gen[OFFSET_Y]->get<int>()+gen[DATE_POS_Y]->get<int>() - h/2 - 3,
                w,
                h,
                gen[BACKGROUND_COLOR]->get<int>()
        );
        tft->drawString(
                ss2.str().c_str(),
                gen[WIDTH]->get<int>()/2+gen[OFFSET_X]->get<int>(),
                gen[HEIGHT]->get<int>()/2+gen[OFFSET_Y]->get<int>()+gen[DATE_POS_Y]->get<int>());
        pDateWidth = w;
        pDay = now.day();
    }

    redraw[TIME] = false;

#ifdef LOG_DETAILED_FRAMETIME
    Log.logf("time and date: %lu", millis()-t5);
    t5 = millis();
#endif

    std::stringstream ss3;
    ss3.precision(gen[DATA_BEGIN_BEGIN + selected[MID] * DATA_SETTINGS_SIZE + DATA_PRECISION_OFFSET]->get<int>());
    ss3 << std::fixed << Data.dataInput[selected[MID]].value << gen[DATA_BEGIN_BEGIN + selected[MID] * DATA_SETTINGS_SIZE + DATA_UNIT_OFFSET]->getString();
    const char* str = ss3.str().c_str();

    int w = textUpdate->textWidth(str);
    int h = textUpdate->fontHeight();
    textUpdate->setColorDepth(8);
    textUpdate->createSprite((gen[NEEDLE_CENTER_OFFSET]->get<int>()-gen[NEEDLE_CENTER_RADIUS]->get<int>())*2, h + SCALE_SPRITE_Y_OFFSET_12, 1);
    textUpdate->fillSprite(gen[BACKGROUND_COLOR]->get<int>());
    textUpdate->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
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

void Gauges::updateStatusBar() {

    bool currentStatus[STATUS_BAR_SIZE];
    for (auto& s : currentStatus) {
        s = false;
    }
    currentStatus[WIFI] = Networking.isWiFiConnected();
    // currentStatus[BT] = Networking.isBLEConnected();
    currentStatus[HEADLIGHTS] = Data.data.engineRunning;
    currentStatus[THROTTLE] = Settings.state.throttleState;

    bool needToRedraw = redraw[STATUS_BAR];
    for(int i = 0; i < STATUS_BAR_SIZE; i++) {
        if(currentStatus[i] != icons[i]) {
            needToRedraw = true;
        }
    }

    if(needToRedraw) {
        tft->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
        tft->setTextDatum(MR_DATUM);
        tft->loadFont("icons16", true);

        std::stringstream ss;
        if(icons[BT])
            ss << STATUS_BT_SYMBOL << " ";
        if(icons[WIFI])
            ss << STATUS_WIFI_SYMBOL << " ";
        if(icons[HEADLIGHTS])
            ss << STATUS_HEADLIGHT_SYMBOL << " ";
        if(icons[THROTTLE])
            ss << STATUS_THROTTLE_SYMBOL << " ";

        std::stringstream ss2;
        if(currentStatus[BT])
            ss2 << STATUS_BT_SYMBOL << " ";
        if(currentStatus[WIFI])
            ss2 << STATUS_WIFI_SYMBOL << " ";
        if(currentStatus[HEADLIGHTS])
            ss2 << STATUS_HEADLIGHT_SYMBOL << " ";
        if(currentStatus[THROTTLE])
            ss2 << STATUS_THROTTLE_SYMBOL << " ";

        std::string string1 = ss.str();
        std::string string2 = ss2.str();

        int w = max(tft->textWidth(string1.c_str()), tft->textWidth(string2.c_str()));
        int h = tft->fontHeight();
        tft->fillRect(
                gen[WIDTH]->get<int>()/2+gen[OFFSET_X]->get<int>()+gen[STATUS_POS_X]->get<int>() - w,
                gen[HEIGHT]->get<int>()/2+gen[OFFSET_Y]->get<int>()+gen[STATUS_POS_Y]->get<int>() - h/2,
                w,
                h,
                gen[BACKGROUND_COLOR]->get<int>()
        );
        tft->drawString(
                string2.c_str(),
                gen[WIDTH]->get<int>()/2+gen[OFFSET_X]->get<int>()+gen[STATUS_POS_X]->get<int>(),
                gen[HEIGHT]->get<int>()/2+gen[OFFSET_Y]->get<int>()+gen[STATUS_POS_Y]->get<int>());

        for(int i = 0; i < STATUS_BAR_SIZE; i++) {
            icons[i] = currentStatus[i];
        }

        redraw[STATUS_BAR] = false;
    }
}


void Gauges::drawSelectedInfo() {

    std::stringstream ss;
    ss << "<- " << std::nouppercase << gen[DATA_BEGIN_BEGIN + selected[LEFT] * DATA_SETTINGS_SIZE + DATA_NAME_OFFSET]->getString() << "\n";
    ss << std::nouppercase << gen[DATA_BEGIN_BEGIN + selected[RIGHT] * DATA_SETTINGS_SIZE + DATA_NAME_OFFSET]->getString() << " ->\n\n\n";
    ss << std::nouppercase << gen[DATA_BEGIN_BEGIN + selected[MID] * DATA_SETTINGS_SIZE + DATA_NAME_OFFSET]->getString() << ":";
    std::string str = ss.str();
    std::transform(str.begin(), str.end(), str.begin(),
                   [](unsigned char c){ return std::tolower(c); });

    textUpdate->setColorDepth(8);
    textUpdate->createSprite(selectedInfoCoords[2], selectedInfoCoords[3], 1);
    textUpdate->fillSprite(gen[BACKGROUND_COLOR]->get<int>());
    textUpdate->setTextColor(gen[FONT_COLOR]->get<int>(), gen[BACKGROUND_COLOR]->get<int>());
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

void Gauges::clearSelectedInfo() {
    tft->fillRect(selectedInfoCoords[0],
                  selectedInfoCoords[1],
                  selectedInfoCoords[2],
                  selectedInfoCoords[3],
                  gen[BACKGROUND_COLOR]->get<int>());
    selectedInfoVisible = false;
}

void Gauges::processEvent(GxFT5436::Event event, void *param) {

    if(event.type != SINGLE_CLICK) return;

    int16_t x = event.x[0];
    int16_t y = event.y[0];

    //change gauge

    Side side = SIDE_SIZE;
    if(x < Settings.general[WIDTH]->get<int>() / 2 - Settings.general[NEEDLE_CENTER_OFFSET]->get<int>())
        side = LEFT;
    else if(x > Settings.general[WIDTH]->get<int>() / 2 + Settings.general[NEEDLE_CENTER_OFFSET]->get<int>())
        side = RIGHT;
    else if(x > Settings.general[WIDTH]->get<int>() / 2 - Settings.general[NEEDLE_CENTER_OFFSET]->get<int>() &&
    x < Settings.general[WIDTH]->get<int>() / 2 + Settings.general[NEEDLE_CENTER_OFFSET]->get<int>() &&
    y > Settings.general[HEIGHT]->get<int>() / 2)
        side = MID;

    if(side != SIDE_SIZE) {
        SettingsClass::DataSource selected[3];
        ((Gauges*)param)->getSelected(selected);

        Log.logf("Current data: %s\n", Settings.dataSourceString[selected[side]].c_str());
        do {
            selected[side] = static_cast<SettingsClass::DataSource>(selected[side] + 1);
            if(selected[side] == SettingsClass::LAST)
                selected[side] = static_cast<SettingsClass::DataSource>(0);
        } while (!Settings.general[DATA_BEGIN_BEGIN + selected[side] * DATA_SETTINGS_SIZE + DATA_ENABLE_OFFSET]->get<bool>());
        Log.logf("Changing to: %s\n", Settings.dataSourceString[selected[side]].c_str());
        ((Gauges*)param)->setSelected(side, selected[side]);
        Settings.saveState();
    }
}

std::vector<Clickable *> *Gauges::getClickables() {
    return &clickables;
}
