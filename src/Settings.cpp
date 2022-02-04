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
    for(int i=0; i<SETTINGS_SIZE; i++)
        settings->general[i]->setDefault();
}

void Settings::init() {

    general[WIFI_SSID] = new Field("SSID", "dlink-74A1");
    general[WIFI_PASS] = new Field("pass", "fdpqg49953");
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


    for(int i=0; i<INPUT_SIZE; i++) {
        general[INPUT_0 + INPUT_SETTINGS_SIZE * i + INPUT_R_OFFSET] = (new Field("r", 0.0f))->setStep(0.1f);
        general[INPUT_0 + INPUT_SETTINGS_SIZE * i + INPUT_TYPE_OFFSET] = (new Field("type", Logarithmic))->setType(LIST);
        general[INPUT_0 + INPUT_SETTINGS_SIZE * i + INPUT_BETA_OFFSET] = (new Field("beta", 0.0f))->setStep(0.1f);
        general[INPUT_0 + INPUT_SETTINGS_SIZE * i + INPUT_R25_OFFSET] = (new Field("r25", 0.0f))->setStep(0.1f);
        general[INPUT_0 + INPUT_SETTINGS_SIZE * i + INPUT_RMIN_OFFSET] = (new Field("r_min", 0.0f))->setStep(0.1f);
        general[INPUT_0 + INPUT_SETTINGS_SIZE * i + INPUT_RMAX_OFFSET] = (new Field("r_max", 0.0f))->setStep(0.1f);
        general[INPUT_0 + INPUT_SETTINGS_SIZE * i + INPUT_MAXVAL_OFFSET] = (new Field("max_val", 0.0f))->setStep(0.1f);
    }

    for(int i=ADS1115_0; i<=ADC_6; i++) {
        general[DATA_0 + DATA_SETTINGS_SIZE * i + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
        general[DATA_0 + DATA_SETTINGS_SIZE * i + DATA_NAME_OFFSET] = new Field("name", "");
        general[DATA_0 + DATA_SETTINGS_SIZE * i + DATA_UNIT_OFFSET] = new Field("unit", "");
        general[DATA_0 + DATA_SETTINGS_SIZE * i + DATA_SCALE_START_OFFSET] = new Field("scale_start", 0.0f);
        general[DATA_0 + DATA_SETTINGS_SIZE * i + DATA_SCALE_END_OFFSET] = new Field("scale_end", 0.0f);
        general[DATA_0 + DATA_SETTINGS_SIZE * i + DATA_PRECISION_OFFSET] = new Field("precision", 1);
        general[DATA_0 + DATA_SETTINGS_SIZE * i + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);
    }

    general[DATA_0 + DATA_SETTINGS_SIZE * VOLTAGE + DATA_ENABLE_OFFSET] = (new Field("enable", 1))->setType(CHECKBOX);
    general[DATA_0 + DATA_SETTINGS_SIZE * VOLTAGE + DATA_NAME_OFFSET] = new Field("name", "Voltage");
    general[DATA_0 + DATA_SETTINGS_SIZE * VOLTAGE + DATA_UNIT_OFFSET] = new Field("unit", "V");
    general[DATA_0 + DATA_SETTINGS_SIZE * VOLTAGE + DATA_SCALE_START_OFFSET] = new Field("scale_start", 6);
    general[DATA_0 + DATA_SETTINGS_SIZE * VOLTAGE + DATA_SCALE_END_OFFSET] = new Field("scale_end", 18);
    general[DATA_0 + DATA_SETTINGS_SIZE * VOLTAGE + DATA_PRECISION_OFFSET] = new Field("precision", 1);
    general[DATA_0 + DATA_SETTINGS_SIZE * VOLTAGE + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_NAME_OFFSET] = new Field("name", "SW");
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_UNIT_OFFSET] = new Field("unit", "°");
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_SCALE_START_OFFSET] = new Field("scale_start", -550);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_SCALE_END_OFFSET] = new Field("scale_end", 550);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_PRECISION_OFFSET] = new Field("precision", 0.0f);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_NAME_OFFSET] = new Field("name", "Speed");
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_UNIT_OFFSET] = new Field("unit", "km/h");
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_SCALE_START_OFFSET] = new Field("scale_start", 0.0f);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_SCALE_END_OFFSET] = new Field("scale_end", 280);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_PRECISION_OFFSET] = new Field("precision", 1);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_RPM + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_RPM + DATA_NAME_OFFSET] = new Field("name", "RPM");
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_RPM + DATA_UNIT_OFFSET] = new Field("unit", "rpm");
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_RPM + DATA_SCALE_START_OFFSET] = new Field("scale_start", 6);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_RPM + DATA_SCALE_END_OFFSET] = new Field("scale_end", 8);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_RPM + DATA_PRECISION_OFFSET] = new Field("precision", 0.0f);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_RPM + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_GAS + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_GAS + DATA_NAME_OFFSET] = new Field("name", "Throttle");
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_GAS + DATA_UNIT_OFFSET] = new Field("unit", "%");
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_GAS + DATA_SCALE_START_OFFSET] = new Field("scale_start", 0.0f);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_GAS + DATA_SCALE_END_OFFSET] = new Field("scale_end", 100);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_GAS + DATA_PRECISION_OFFSET] = new Field("precision", 0.1f);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_GAS + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_HB + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_HB + DATA_NAME_OFFSET] = new Field("name", "Handbrake");
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_HB + DATA_UNIT_OFFSET] = new Field("unit", "");
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_HB + DATA_SCALE_START_OFFSET] = new Field("scale_start", 0.0f);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_HB + DATA_SCALE_END_OFFSET] = new Field("scale_end", 1);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_HB + DATA_PRECISION_OFFSET] = new Field("precision", 0.0f);
    general[DATA_0 + DATA_SETTINGS_SIZE * CAN_HB + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

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
        } else {
            for(int i=0; i<SETTINGS_SIZE; i++) {
                if(settings->general[i]->isConfigurable()) {
                    switch (general[i]->getType()) {
                        case STRING: {
                            char c[64] = "";
                            strcpy((char *)c, doc[i]);
                            general[i]->set((const char*)c);
#ifdef LOG_SETTINGS
                            Log.logf("%s: %s\n", general[i]->getName().c_str(), general[i]->getString().c_str());
#endif
                            break;
                        }
                        default: {
                            float f = doc[i].as<float>();
                            general[i]->set(f);
#ifdef LOG_SETTINGS
                            Log.logf("%s: %f\n", general[i]->getName().c_str(), general[i]->get<float>());
#endif
                            break;
                        }
                    }
                }
            }
        }

        file.close();


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

    for(int i=0; i<SETTINGS_SIZE; i++) {
        if(settings->general[i]->isConfigurable()) {
            switch (general[i]->getType()) {
                case STRING: {
                    doc[i] = (String)general[i]->getString().c_str();
#ifdef LOG_SETTINGS
                    Log.logf("%s: %s\n", general[i]->getName().c_str(), general[i]->getString().c_str());
#endif
                    break;
                }
                default: {
                    doc[i] = general[i]->get<float>();
#ifdef LOG_SETTINGS
                    Log.logf("%s: %f\n", general[i]->getName().c_str(), general[i]->get<float>());
#endif
                    break;
                }
            }
        }
    }

    if (serializeJson(doc, file) == 0) {
        Log.log("Failed to write to file");
    }
    Log.log("Settings saved");

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

    file.close();
}
