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

    visual.antialiasing = true;
	visual.dark = true;
	visual.drawMainScaleLine = false;

	visual.width = 480;
	visual.height = 320;

	visual.offsetX = -2; //17
	visual.offsetY = -4; //29


	visual.ellipseA = 216;
	visual.ellipseB = 116;

	visual.promptWidth = 330;
	visual.promptHeight = 120;

	visual.needleCenterRadius = 20;
	visual.needleCenterOffset = 75;
	visual.needleLength = 112; //97
	visual.needleTopWidth = 2;
	visual.needleBottomWidth = 10;

	visual.timePosY = -60; //Centre centre
	visual.timeSize = 36;
	visual.datePosY = -35; //Centre centre
	visual.dateSize = 16;
	visual.scaleSize = 12;


//	int16_t sscaleLeft[] = 	{30, 60, 90, 120, 150};
//	int16_t sscaleLeft[] = 	{0, 2000, 4000, 6000, 8000};
//	int16_t sscaleRight[] = 	{6, 9, 12, 15, 18};
	int16_t sscaleXLeft[] = 	{-0, 	-75, 	-100, 	-75, 	-0};
	int16_t sscaleXRight[] = 	{0, 	75, 	100, 	75, 	0};
	int16_t sscaleY[] = 		{75, 	45, 	0,		-45, 	-75};

	for(int i=0; i<5; i++){
//		visual.scaleLeft[i] = sscaleLeft[i];
//		visual.scaleRight[i] = sscaleRight[i];
		visual.scaleXLeft[i] = sscaleXLeft[i];
		visual.scaleXRight[i] = sscaleXRight[i];
		visual.scaleY[i] = sscaleY[i];
	}

	visual.scaleMainWidth = 7;
	visual.scaleLargeWidth = 4;
	visual.scaleSmallWidth = 2;
	visual.scaleLargeLength = 17;
	visual.scaleSmallLength = 7;
	visual.scaleLargeSteps = 12;
	visual.scaleSmallSteps = 4;
	visual.scaleAccColorEvery = 8;
	visual.scaleTextSteps = 4;
	visual.scaleAntialiasing = 4;
	visual.internalEllipseDistance = 10;

	visual.backgroundColor = TFT_WHITE;
	visual.scaleColor = TFT_GREEN;
	visual.scaleAccColor = TFT_RED;
	visual.needleCenterColor = 0x31A6;
	visual.fontColor = visual.scaleColor;
	visual.iconColor = visual.scaleColor;
	visual.needleColor = TFT_RED;

	if(visual.dark) {
		visual.scaleColor = TFT_GREEN;
		visual.backgroundColor= TFT_BLACK;
		visual.needleCenterColor = TFT_DARKGREY;
 	}

	visual.needleCenterColor = 0x31A6;

	for(int i=0; i<6; i++) {
        input[i].enable = false;
        strcpy((char *)(input[i].name), "");
        strcpy((char *)(input[i].unit), "u");
        input[i].scaleStart = 0;
        input[i].scaleEnd = 0;
        input[i].r = 0;
        input[i].type = Logarithmic;
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
        StaticJsonDocument<4*1024> doc;
        fs::File file = SPIFFS.open("/settings.json", "r");
        DeserializationError error = deserializeJson(doc, file);
        if (error) {
            Serial.println(F("Failed to read file, using default configuration"));
            loadDefault();
        }

        visual.offsetX = doc["offsetX"] | 0;
        visual.offsetY = doc["offsetY"] | 0;
        visual.ellipseA = doc["ellipseA"] | 0;
        visual.ellipseB = doc["ellipseB"] | 0;
        visual.needleCenterRadius = doc["needleCenterRadius"] | 0;
        visual.needleCenterOffset = doc["needleCenterOffset"] | 0;
        visual.needleLength = doc["needleLength"] | 0;
        visual.needleBottomWidth = doc["needleBottomWidth"] | 0;
        visual.needleTopWidth = doc["needleTopWidth"] | 0;

        for(int i=0; i<6; i++) {
            input[i].enable =       doc["input_" + (String)i + "_en"] | 0;
            strcpy((char *)settings->input[i].name, doc["input_" + (String)i + "_name"] | "");
            strcpy((char *)settings->input[i].unit, doc["input_" + (String)i + "_unit"] | "u");
            input[i].scaleStart =   doc["input_" + (String)i + "_scaleStart"] | 0;
            input[i].scaleEnd =     doc["input_" + (String)i + "_scaleEnd"] | 0;
            input[i].r =            doc["input_" + (String)i + "_rballance"].as<float>();
            input[i].type =         doc["input_" + (String)i + "_type"] | Linear;
            input[i].beta =         doc["input_" + (String)i + "_beta"].as<float>();
            input[i].r25 =          doc["input_" + (String)i + "_r25"].as<float>();
            input[i].rmin =         doc["input_" + (String)i + "_rmin"].as<float>();
            input[i].rmax =         doc["input_" + (String)i + "_rmax"].as<float>();
            input[i].maxVal =       doc["input_" + (String)i + "_max_val"].as<float>();
        }

        file.close();

        Serial.println("Read from JSON: ");

        Serial.println("[offsetX]" + (String)visual.offsetX);
        Serial.println("[offsetY]" + (String)visual.offsetY);
        Serial.println("[ellipseA]" + (String)visual.ellipseA);
        Serial.println("[ellipseB]" + (String)visual.ellipseB);
        Serial.println("[needleCenterRadius]" + (String)visual.needleCenterRadius);
        Serial.println("[needleCenterOffset]" + (String)visual.needleCenterOffset);
        Serial.println("[needleLength]" + (String)visual.needleLength);
        Serial.println("[needleBottomWidth]" + (String)visual.needleBottomWidth);
        Serial.println("[needleTopWidth]" + (String)visual.needleTopWidth);

        for(int i=0; i<6; i++) {
            Serial.println("[input_" + (String)i + ".enable]" + (String)input[i].enable);
            Serial.println("[input_" + (String)i + ".name]" + (String)(char *)input[i].name);
            Serial.println("[input_" + (String)i + ".unit]" + (String)(char *)input[i].unit);
            Serial.println("[input_" + (String)i + ".scaleStart]" + (String)input[i].scaleStart);
            Serial.println("[input_" + (String)i + ".scaleEnd]" + (String)input[i].scaleEnd);
            Serial.println("[input_" + (String)i + ".r]" + (String)input[i].r);
            Serial.println("[input_" + (String)i + ".type]" + (String)input[i].type);
            Serial.println("[input_" + (String)i + ".beta]" + (String)input[i].beta);
            Serial.println("[input_" + (String)i + ".r25]" + (String)input[i].r25);
            Serial.println("[input_" + (String)i + ".rmin]" + (String)input[i].rmin);
            Serial.println("[input_" + (String)i + ".rmax]" + (String)input[i].rmax);
            Serial.println("[input_" + (String)i + ".maxVal]" + (String)input[i].maxVal);
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
    StaticJsonDocument<4*1024> doc;

    doc["offsetX"] = visual.offsetX;
    doc["offsetY"] = visual.offsetY;
    doc["ellipseA"] = visual.ellipseA;
    doc["ellipseB"] = visual.ellipseB;
    doc["needleCenterRadius"] = visual.needleCenterRadius;
    doc["needleCenterOffset"] = visual.needleCenterOffset;
    doc["needleLength"] = visual.needleLength;
    doc["needleBottomWidth"] = visual.needleBottomWidth;
    doc["needleTopWidth"] = visual.needleTopWidth;
    for(int i=0; i<6; i++) {
        doc["input_" + (String)i + "_en"] = input[i].enable;
        doc["input_" + (String)i + "_name"] = (String)(char*)input[i].name;
        doc["input_" + (String)i + "_unit"] = (String)(char*)input[i].unit;
        doc["input_" + (String)i + "_scaleStart"] = input[i].scaleStart;
        doc["input_" + (String)i + "_scaleEnd"] = input[i].scaleEnd;
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