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

	dataDisplay[CAN_STEERING_ANGLE].enable = false;
	strcpy((char *)(dataDisplay[CAN_STEERING_ANGLE].name), "SW");
	strcpy((char *)(dataDisplay[CAN_STEERING_ANGLE].unit), "°");
	dataDisplay[CAN_STEERING_ANGLE].scaleStart = -550;
	dataDisplay[CAN_STEERING_ANGLE].scaleEnd = 550; //?

	dataDisplay[CAN_SPEED].enable = false;
	strcpy((char *)(dataDisplay[CAN_SPEED].name), "Speed");
	strcpy((char *)(dataDisplay[CAN_SPEED].unit), "km/h");
	dataDisplay[CAN_SPEED].scaleStart = 0;
	dataDisplay[CAN_SPEED].scaleEnd = 280;

	dataDisplay[CAN_RPM].enable = false;
	strcpy((char *)(dataDisplay[CAN_RPM].name), "RPM");
	strcpy((char *)(dataDisplay[CAN_RPM].unit), "rpm");
	dataDisplay[CAN_RPM].scaleStart = 0;
	dataDisplay[CAN_RPM].scaleEnd = 8;

	dataDisplay[CAN_GAS].enable = false;
	strcpy((char *)(dataDisplay[CAN_GAS].name), "Throttle");
	strcpy((char *)(dataDisplay[CAN_GAS].unit), "%");
	dataDisplay[CAN_GAS].scaleStart = 0;
	dataDisplay[CAN_GAS].scaleEnd = 100;

	dataDisplay[CAN_HB].enable = false;
	strcpy((char *)(dataDisplay[CAN_HB].name), "HB");
	strcpy((char *)(dataDisplay[CAN_HB].unit), "");
	dataDisplay[CAN_HB].scaleStart = 0;
	dataDisplay[CAN_HB].scaleEnd = 1;


}

void Settings::init() {

    general[SSID] = new Field("SSID", "dlink-74A1");
    general[PASS] = new Field("pass", "fdpqg49953");
    general[WIDTH] = new Field("width", 480, false);
    general[HEIGHT] = new Field("height", 320, false);
    general[OFFSET_Y] = new Field("offset_y", -2);
    general[OFFSET_X] = new Field("offset_x", -4);
    general[ELLIPSE_A] = new Field("ellipse_A", 216);
    general[ELLIPSE_B] = new Field("ellipse_B", 116);
    general[PROMPT_WIDTH] = new Field("prompt_width", 330);
    general[PROMPT_HEIGHT] = new Field("prompt_height", 120);

    general[NEEDLE_CENTER_RADIUS] = new Field("needle_center_radius", 20);
    general[NEEDLE_CENTER_OFFSET] = new Field("needle_center_offset", 75);
    general[NEEDLE_LENGTH] = new Field("needle_length", 112);
    general[NEEDLE_TOP_WIDTH] = new Field("needle_top_width", 2);
    general[NEEDLE_BOTTOM_WIDTH] = new Field("needle_bottom_width", 10);

    general[TIME_POS_Y] = new Field("time_pos_Y", -60);
    general[TIME_SIZE] = new Field("time_size", 36, false);
    general[DATE_POS_Y] = new Field("date_pos_Y", -35);
    general[DATE_SIZE] = new Field("date_size", 16, false);
    general[SCALE_SIZE] = new Field("scale_size", 12, false);

    general[SCALE_MAIN_WIDTH] = new Field("scale_main_width", 7);
    general[SCALE_LARGE_WIDTH] = new Field("scale_large_width", 4);
    general[SCALE_SMALL_WIDTH] = new Field("scale_small_width", 2);
    general[SCALE_LARGE_LENGTH] = new Field("scale_large_length", 17);
    general[SCALE_SMALL_LENGTH] = new Field("scale_small_length", 7);
    general[SCALE_LARGE_STEPS] = new Field("scale_large_steps", 12);
    general[SCALE_SMALL_STEPS] = new Field("scale_small_steps", 4);
    general[SCALE_ACC_COLOR_EVERY] = new Field("scale_acc_color_every", 8);
    general[SCALE_TEXT_STEPS] = new Field("scale_text_steps", 4);
    general[SCALE_TEXT_OFFSET] = new Field("scale_text_offset", 35);

    general[BACKGROUND_COLOR] = new Field("background_color", (float)TFT_BLACK);
    general[SCALE_COLOR] = new Field("scale_color", TFT_GREEN);
    general[SCALE_ACC_COLOR] = new Field("scale_acc_color", TFT_RED);
    general[NEEDLE_CENTER_COLOR] = new Field("needle_center_color", 0x31A6);
    general[FONT_COLOR] = new Field("font_color", TFT_GREEN);
    general[ICON_COLOR] = new Field("icon_color", TFT_GREEN);
    general[NEEDLE_COLOR] = new Field("needle_color", TFT_RED);

//    for(int i=HEIGHT+1; i<FIELDS_SIZE; i++) {
//        fields[i] = new Field("as", (float)i);
//    }

//    for(int i=0; i<FIELDS_SIZE; i++) {
//        fields[i]->setDefault();
//    }

//    Log.logf("%s: %d\n", fields[OFFSET_Y]->getName().c_str(), fields[OFFSET_Y]->get<int>());
//    Log.logf("%s: %f\n", fields[OFFSET_Y]->getName().c_str(), fields[OFFSET_Y]->get<float>());
//    Log.logf("%s: %s\n", fields[OFFSET_Y]->getName().c_str(), fields[OFFSET_Y]->get<bool>() ? "true" : "false");
//
//    Log.logf("%s: %s\n", fields[OFFSET_X]->getName().c_str(), fields[OFFSET_X]->getString().c_str());
////    fields[OFFSET_X]->set("jan pawel");
//    Log.logf("%s: %s\n", fields[OFFSET_X]->getName().c_str(), fields[OFFSET_X]->getString().c_str());
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


        for(int i=0; i<FIELDS_SIZE; i++) {
            switch (general[i]->getType()) {
                case FLOAT: {
                    float f = doc[general[i]->getName()] | general[i]->getDefault<float>();
                    general[i]->set(f);
                    Log.logf("%s: %f\n", general[i]->getName().c_str(), general[i]->get<float>());
                    break;
                }
                case STRING: {
                    char c[64] = "";
                    strcpy((char *)c, doc[general[i]->getName()] | general[i]->getDefaultString().c_str());
                    general[i]->set((const char*)c);
                    Log.logf("%s: %s\n", general[i]->getName().c_str(), general[i]->getString().c_str());
                    break;
                }
            }
//            switch (fields[i]->getType()) {
//                case FLOAT: Log.logf("%s: %f\n", fields[i]->getName().c_str(), fields[i]->get<float>()); break;
//                case STRING: Log.logf("%s: %s\n", fields[i]->getName().c_str(), fields[i]->getString().c_str()); break;
//            }
        }

//        for(int i=0; i<FIELDS_SIZE; i++) {
//
//        }

//        strcpy((char *)settings->general.ssid, doc["ssid"] | "");
//        strcpy((char *)settings->general.pass, doc["pass"] | "");
//        visual.offsetX = doc["offsetX"]. | 0;
//        visual.offsetY = doc["offsetY"] | 0;
//        visual.ellipseA = doc["ellipseA"] | 0;
//        visual.ellipseB = doc["ellipseB"] | 0;
//        visual.needleCenterRadius = doc["needleCenterRadius"] | 0;
//        visual.needleCenterOffset = doc["needleCenterOffset"] | 0;
//        visual.needleLength = doc["needleLength"] | 0;
//        visual.needleBottomWidth = doc["needleBottomWidth"] | 0;
//        visual.needleTopWidth = doc["needleTopWidth"] | 0;

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
    Log.log("Saving settings");
    if(SPIFFS.exists("/settings.json"))
        SPIFFS.remove("/settings.json");
    fs::File file = SPIFFS.open("/settings.json", "w");
    StaticJsonDocument<4*1024> doc;

    for(int i=0; i<FIELDS_SIZE; i++) {
        switch (general[i]->getType()) {
            case FLOAT: {
                doc[general[i]->getName()] = general[i]->get<float>();
                Log.logf("%s: %f\n", general[i]->getName().c_str(), general[i]->get<float>());
                break;
            }
            case STRING: {
                doc[general[i]->getName()] = (String)general[i]->getString().c_str();
                Log.logf("%s: %s\n", general[i]->getName().c_str(), general[i]->getString().c_str());
                break;
            }
        }
    }


//    doc["ssid"] = (String)(char*)general.ssid;
//    doc["pass"] = (String)(char*)general.pass;
//    doc["offsetX"] = visual.offsetX;
//    doc["offsetY"] = visual.offsetY;
//    doc["ellipseA"] = visual.ellipseA;
//    doc["ellipseB"] = visual.ellipseB;
//    doc["needleCenterRadius"] = visual.needleCenterRadius;
//    doc["needleCenterOffset"] = visual.needleCenterOffset;
//    doc["needleLength"] = visual.needleLength;
//    doc["needleBottomWidth"] = visual.needleBottomWidth;
//    doc["needleTopWidth"] = visual.needleTopWidth;
//    for(int i=0; i<6; i++) {
//        doc["input_" + (String)i + "_rballance"] = input[i].r;
//        doc["input_" + (String)i + "_type"] = input[i].type;
//        doc["input_" + (String)i + "_beta"] = input[i].beta;
//        doc["input_" + (String)i + "_r25"] = input[i].r25;
//        doc["input_" + (String)i + "_rmin"] = input[i].rmin;
//        doc["input_" + (String)i + "_rmax"] = input[i].rmax;
//        doc["input_" + (String)i + "_max_val"] = input[i].maxVal;
//    }
//    for(int i=ADS1115_0; i<LAST; i++) {
//        doc["dataDisplay_" + (String)i + "_en"] = dataDisplay[i].enable;
//        doc["dataDisplay_" + (String)i + "_name"] = (String)(char*)dataDisplay[i].name;
//        doc["dataDisplay_" + (String)i + "_unit"] = (String)(char*)dataDisplay[i].unit;
//        doc["dataDisplay_" + (String)i + "_scaleStart"] = dataDisplay[i].scaleStart;
//        doc["dataDisplay_" + (String)i + "_scaleEnd"] = dataDisplay[i].scaleEnd;
//    }
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
