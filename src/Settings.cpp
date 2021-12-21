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

    strcpy((char *)(general.ssid), "ESP32");
    strcpy((char *)(general.pass), "12345678");
    strcpy((char *)(general.ssid), "dlink-74A1");
    strcpy((char *)(general.pass), "fdpqg49953");

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

	visual.scaleMainWidth = 7;
	visual.scaleLargeWidth = 4;
	visual.scaleSmallWidth = 2;
	visual.scaleLargeLength = 17;
	visual.scaleSmallLength = 7;
	visual.scaleLargeSteps = 12;
	visual.scaleSmallSteps = 4;
	visual.scaleAccColorEvery = 8;
	visual.scaleTextSteps = 4;
	visual.scaleTextOffset = 35;
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
        input[i].r = 0;
        input[i].type = Logarithmic;
        input[i].beta = 0;
        input[i].r25 = 0;
        input[i].rmin = 0;
        input[i].rmax = 0;
        input[i].maxVal = 0;
	}

	for(int i=ADS1115_0; i<=ADC_6; i++) {
	    dataDisplay[i].enable = false;
	    strcpy((char *)(dataDisplay[i].name), "");
	    strcpy((char *)(dataDisplay[i].unit), "u");
	    dataDisplay[i].scaleStart = 0;
	    dataDisplay[i].scaleEnd = 0;
	}

	dataDisplay[VOLTAGE].enable = true;
	strcpy((char *)(dataDisplay[VOLTAGE].name), "Voltage");
	strcpy((char *)(dataDisplay[VOLTAGE].unit), "V");
	dataDisplay[VOLTAGE].scaleStart = 6;
	dataDisplay[VOLTAGE].scaleEnd = 18;

	dataDisplay[CAN_RPM].enable = false;
	strcpy((char *)(dataDisplay[CAN_RPM].name), "RPM");
	strcpy((char *)(dataDisplay[CAN_RPM].unit), "1000/min");
	dataDisplay[CAN_RPM].scaleStart = 0;
	dataDisplay[CAN_RPM].scaleEnd = 8;

}

void Settings::load() {
    Log.log("Loading settings");
    if(SPIFFS.exists("/settings.json")) {
        StaticJsonDocument<4*1024> doc;
        fs::File file = SPIFFS.open("/settings.json", "r");
        DeserializationError error = deserializeJson(doc, file);
        if (error) {
            Log.log("Failed to read file, using default configuration");
            loadDefault();
        }

        strcpy((char *)settings->general.ssid, doc["ssid"] | "");
        strcpy((char *)settings->general.pass, doc["pass"] | "");
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
            input[i].r =            doc["input_" + (String)i + "_rballance"].as<float>();
            input[i].type =         doc["input_" + (String)i + "_type"] | Linear;
            input[i].beta =         doc["input_" + (String)i + "_beta"].as<float>();
            input[i].r25 =          doc["input_" + (String)i + "_r25"].as<float>();
            input[i].rmin =         doc["input_" + (String)i + "_rmin"].as<float>();
            input[i].rmax =         doc["input_" + (String)i + "_rmax"].as<float>();
            input[i].maxVal =       doc["input_" + (String)i + "_max_val"].as<float>();
        }

        for(int i=ADS1115_0; i<LAST; i++) {
            dataDisplay[i].enable =       doc["dataDisplay_" + (String)i + "_en"] | 0;
            strcpy((char *)settings->dataDisplay[i].name, doc["dataDisplay_" + (String)i + "_name"] | "");
            strcpy((char *)settings->dataDisplay[i].unit, doc["dataDisplay_" + (String)i + "_unit"] | "u");
            dataDisplay[i].scaleStart =   doc["dataDisplay_" + (String)i + "_scaleStart"] | 0;
            dataDisplay[i].scaleEnd =     doc["dataDisplay_" + (String)i + "_scaleEnd"] | 0;
        }

        file.close();

#ifdef LOG_LOADED_SETTINGS

        Log.log("Read from JSON: ");

        Log.log("[offsetX]" + (String)visual.offsetX);
        Log.log("[offsetY]" + (String)visual.offsetY);
        Log.log("[ellipseA]" + (String)visual.ellipseA);
        Log.log("[ellipseB]" + (String)visual.ellipseB);
        Log.log("[needleCenterRadius]" + (String)visual.needleCenterRadius);
        Log.log("[needleCenterOffset]" + (String)visual.needleCenterOffset);
        Log.log("[needleLength]" + (String)visual.needleLength);
        Log.log("[needleBottomWidth]" + (String)visual.needleBottomWidth);
        Log.log("[needleTopWidth]" + (String)visual.needleTopWidth);

        for(int i=0; i<6; i++) {
            Log.log("[input_" + (String)i + ".r]" + (String)input[i].r);
            Log.log("[input_" + (String)i + ".type]" + (String)input[i].type);
            Log.log("[input_" + (String)i + ".beta]" + (String)input[i].beta);
            Log.log("[input_" + (String)i + ".r25]" + (String)input[i].r25);
            Log.log("[input_" + (String)i + ".rmin]" + (String)input[i].rmin);
            Log.log("[input_" + (String)i + ".rmax]" + (String)input[i].rmax);
            Log.log("[input_" + (String)i + ".maxVal]" + (String)input[i].maxVal);
        }

        for(int i=ADS1115_0; i<LAST; i++) {
            Log.log("[dataDisplay_" + (String)i + ".enable]" + (String)dataDisplay[i].enable);
            Log.log("[dataDisplay_" + (String)i + ".name]" + (String)(char *)dataDisplay[i].name);
            Log.log("[dataDisplay_" + (String)i + ".unit]" + (String)(char *)dataDisplay[i].unit);
            Log.log("[dataDisplay_" + (String)i + ".scaleStart]" + (String)dataDisplay[i].scaleStart);
            Log.log("[dataDisplay_" + (String)i + ".scaleEnd]" + (String)dataDisplay[i].scaleEnd);
        }
        Log.log("End of read");

#endif

    } else {
        Log.log("File not found. Loading defaults");
        loadDefault();
    }

}

void Settings::save() {
    if(SPIFFS.exists("/settings.json"))
        SPIFFS.remove("/settings.json");
    fs::File file = SPIFFS.open("/settings.json", "w");
    StaticJsonDocument<4*1024> doc;


    doc["ssid"] = (String)(char*)general.ssid;
    doc["pass"] = (String)(char*)general.pass;
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
        doc["input_" + (String)i + "_rballance"] = input[i].r;
        doc["input_" + (String)i + "_type"] = input[i].type;
        doc["input_" + (String)i + "_beta"] = input[i].beta;
        doc["input_" + (String)i + "_r25"] = input[i].r25;
        doc["input_" + (String)i + "_rmin"] = input[i].rmin;
        doc["input_" + (String)i + "_rmax"] = input[i].rmax;
        doc["input_" + (String)i + "_max_val"] = input[i].maxVal;
    }
    for(int i=ADS1115_0; i<LAST; i++) {
        doc["dataDisplay_" + (String)i + "_en"] = dataDisplay[i].enable;
        doc["dataDisplay_" + (String)i + "_name"] = (String)(char*)dataDisplay[i].name;
        doc["dataDisplay_" + (String)i + "_unit"] = (String)(char*)dataDisplay[i].unit;
        doc["dataDisplay_" + (String)i + "_scaleStart"] = dataDisplay[i].scaleStart;
        doc["dataDisplay_" + (String)i + "_scaleEnd"] = dataDisplay[i].scaleEnd;
    }
    if (serializeJson(doc, file) == 0) {
        Log.log("Failed to write to file");
    }
    Log.log("Settings saved");

    // Close the file
    file.close();
}

void Settings::loadSelected(Settings::DataSource *selected) {
    Log.log("Loading selected");
    if(SPIFFS.exists("/selected.json")) {
        StaticJsonDocument<128> doc;
        fs::File file = SPIFFS.open("/selected.json", "r");
        DeserializationError error = deserializeJson(doc, file);
        if (error) {
            Log.log("Failed to read file, using default configuration");
        }

        selected[0] = doc["sel_0"] | VOLTAGE;
        selected[1] = doc["sel_1"] | VOLTAGE;
        selected[2] = doc["sel_2"] | VOLTAGE;
    } else {
        selected[0] = VOLTAGE;
        selected[1] = VOLTAGE;
        selected[2] = VOLTAGE;
    }
}

void Settings::saveSelected(Settings::DataSource *selected) {
    if(SPIFFS.exists("/selected.json"))
        SPIFFS.remove("/selected.json");
    fs::File file = SPIFFS.open("/selected.json", "w");
    StaticJsonDocument<128> doc;

    doc["sel_0"] = selected[0];
    doc["sel_1"] = selected[1];
    doc["sel_2"] = selected[2];

    if (serializeJson(doc, file) == 0) {
        Log.log("Failed to write to file");
    }
    Log.log("Selected saved");

    // Close the file
    file.close();
}
