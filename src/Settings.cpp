#include "Settings.h"

Settings* Settings::settings = NULL;

Settings::Settings() {

}

Settings* Settings::getInstance()
{
    if(settings==nullptr)
        settings = new Settings();
    return settings;
}



void Settings::loadDefault() {

	antialiasing = true;
	dark = true;
	drawMainScaleLine = false;

	width = 480;
	height = 320;

	offsetX = -2; //17
	offsetY = -4; //29


	ellipseA = 216;
	ellipseB = 116;

	needleCenterRadius = 20;
	needleCenterOffset = 75;
	needleLength = 112; //97
	needleTopWidth = 2;
	needleBottomWidth = 10;

	timePosY = -60; //Centre centre
	timeSize = 36;
	datePosY = -35; //Centre centre
	dateSize = 16;
	scaleSize = 12;

	scaleStart = 30;
	scaleEnd = 150;

	int16_t sscaleLeft[] = 	{30, 60, 90, 120, 150};
//	int16_t sscaleLeft[] = 	{0, 2000, 4000, 6000, 8000};
	int16_t sscaleRight[] = 	{6, 9, 12, 15, 18}; 
	int16_t sscaleXLeft[] = 	{-0, 	-75, 	-100, 	-75, 	-0};
	int16_t sscaleXRight[] = 	{0, 	75, 	100, 	75, 	0};
	int16_t sscaleY[] = 		{75, 	45, 	0,		-45, 	-75};

	for(int i=0; i<5; i++){
		scaleLeft[i] = sscaleLeft[i];
		scaleRight[i] = sscaleRight[i];
		scaleXLeft[i] = sscaleXLeft[i];
		scaleXRight[i] = sscaleXRight[i];
		scaleY[i] = sscaleY[i];
	}
	

	scaleMainWidth = 7;
	scaleLargeWidth = 4;
	scaleSmallWidth = 2;
	scaleLargeLength = 17;
	scaleSmallLength = 7;
	scaleLargeSteps = 12;
	scaleSmallSteps = 4;
	scaleAccColorEvery = 8;
	scaleTextSteps = 4;
	scaleAntialiasing = 4;
	internalEllipseDistance = 10;

	backgroundColor = TFT_WHITE;
	scaleColor = TFT_GREEN;
	scaleAccColor = TFT_RED;
	needleCenterColor = 0x31A6;
	fontColor = scaleColor;
	iconColor = scaleColor;
	needleColor = TFT_RED;

	if(dark) {
		scaleColor = TFT_GREEN;
		backgroundColor= TFT_BLACK;
		needleCenterColor = TFT_DARKGREY;
 	}

	needleCenterColor = 0x31A6;

	for(int i=0; i<6; i++) {
        input[i].enable = false;
        input[i].r = 0;
        input[i].type = 0;
        input[i].beta = 0;
        input[i].r25 = 0;
        input[i].rmin = 0;
        input[i].rmax = 0;
        input[i].maxVal = 0;
	}


}

void Settings::load() {
    Serial.println("Loading settings");
    if(SPIFFS.exists("/settings.json")) {
        StaticJsonDocument<2048> doc;
        fs::File file = SPIFFS.open("/settings.json", "r");
        DeserializationError error = deserializeJson(doc, file);
        if (error) {
            Serial.println(F("Failed to read file, using default configuration"));
            loadDefault();
        }

        offsetX = doc["offsetX"] | 0;
        offsetY = doc["offsetY"] | 0;
        ellipseA = doc["ellipseA"] | 0;
        ellipseB = doc["ellipseB"] | 0;
        needleCenterRadius = doc["needleCenterRadius"] | 0;
        needleCenterOffset = doc["needleCenterOffset"] | 0;
        needleLength = doc["needleLength"] | 0;
        needleBottomWidth = doc["needleBottomWidth"] | 0;
        needleTopWidth = doc["needleTopWidth"] | 0;

        for(int i=0; i<6; i++) {
            input[i].enable =   doc["input_" + (String)i + "_en"] | 0;
            input[i].r =        doc["input_" + (String)i + "_rballance"] | 0;
            input[i].type =     doc["input_" + (String)i + "_type"] | 0;
            input[i].beta =     doc["input_" + (String)i + "_beta"] | 0;
            input[i].r25 =      doc["input_" + (String)i + "_r25"] | 0;
            input[i].rmin =     doc["input_" + (String)i + "_rmin"] | 0;
            input[i].rmax =     doc["input_" + (String)i + "_rmax"] | 0;
            input[i].maxVal =   doc["input_" + (String)i + "_max_val"] | 0;
        }

        file.close();

        Serial.println("Read from JSON: ");

        Serial.println("[offsetX]" + (String)settings->offsetX);
        Serial.println("[offsetY]" + (String)settings->offsetY);
        Serial.println("[ellipseA]" + (String)settings->ellipseA);
        Serial.println("[ellipseB]" + (String)settings->ellipseB);
        Serial.println("[needleCenterRadius]" + (String)settings->needleCenterRadius);
        Serial.println("[needleCenterOffset]" + (String)settings->needleCenterOffset);
        Serial.println("[needleLength]" + (String)settings->needleLength);
        Serial.println("[needleBottomWidth]" + (String)settings->needleBottomWidth);
        Serial.println("[needleTopWidth]" + (String)settings->needleTopWidth);

        for(int i=0; i<6; i++) {
            Serial.println("[input_" + (String)i + ".enable]" + (String)settings->input[i].enable);
            Serial.println("[input_" + (String)i + ".r]" + (String)settings->input[i].r);
            Serial.println("[input_" + (String)i + ".type]" + (String)settings->input[i].type);
            Serial.println("[input_" + (String)i + ".beta]" + (String)settings->input[i].beta);
            Serial.println("[input_" + (String)i + ".r25]" + (String)settings->input[i].r25);
            Serial.println("[input_" + (String)i + ".rmin]" + (String)settings->input[i].rmin);
            Serial.println("[input_" + (String)i + ".rmax]" + (String)settings->input[i].rmax);
            Serial.println("[input_" + (String)i + ".maxVal]" + (String)settings->input[i].maxVal);
        }
        Serial.println("End of read");
    } else {
        Serial.println("File not found. Loading defaults");
        loadDefault();
    }

}

void Settings::save() {
    if(SPIFFS.exists("/settings.json"))
        SPIFFS.remove("/settings.json");
    fs::File file = SPIFFS.open("/settings.json", "w");
    StaticJsonDocument<2048> doc;
    doc["offsetX"] = offsetX;
    doc["offsetY"] = offsetY;
    doc["ellipseA"] = ellipseA;
    doc["ellipseB"] = ellipseB;
    doc["needleCenterRadius"] = needleCenterRadius;
    doc["needleCenterOffset"] = needleCenterOffset;
    doc["needleLength"] = needleLength;
    doc["needleBottomWidth"] = needleBottomWidth;
    doc["needleTopWidth"] = needleTopWidth;
    for(int i=0; i<6; i++) {
        doc["input_" + (String)i + "_en"] = input[i].enable;
        doc["input_" + (String)i + "_rballance"] = input[i].r;
        doc["input_" + (String)i + "_type"] = input[i].type;
        doc["input_" + (String)i + "_beta"] = input[i].beta;
        doc["input_" + (String)i + "_r25"] = input[i].r25;
        doc["input_" + (String)i + "_rmin"] = input[i].rmin;
        doc["input_" + (String)i + "_rmax"] = input[i].rmax;
        doc["input_" + (String)i + "_max_val"] = input[i].maxVal;
    }
    if (serializeJson(doc, file) == 0) {
        Serial.println(F("Failed to write to file"));
    }
    Serial.println(F("Settings saved"));

    // Close the file
    file.close();
}