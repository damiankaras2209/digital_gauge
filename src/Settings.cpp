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

#ifdef LOG_SETTINGS
    Log.logf("GENERAL_SETTINGS_SIZE: %d\n", GENERAL_SETTINGS_SIZE);
    Log.logf("INPUT_END_END: %d\n", INPUT_END_END);
    Log.logf("INPUT_BEGIN_BEGIN: %d\n", INPUT_BEGIN_BEGIN);
    Log.logf("INPUT_SETTINGS_SIZE: %d\n", INPUT_SETTINGS_SIZE);
    Log.logf("INPUT_SIZE: %d\n", INPUT_SIZE);
    Log.logf("DATA_SETTINGS_SIZE: %d\n", DATA_SETTINGS_SIZE);
    Log.logf("DATA_SIZE: %d\n", DATA_SIZE);
    Log.logf("SETTINGS_SIZE: %d\n", SETTINGS_SIZE);
#endif

    general[DEMO] = (new Field("demo", 0.0f))->setType(CHECKBOX);
    general[WIFI_SSID] = new Field("SSID", "dlink-74A1");
    general[WIFI_PASS] = new Field("pass", "fdpqg49953");
    general[WIDTH] = new Field("width", 480, false);
    general[HEIGHT] = new Field("height", 320, false);
    general[OFFSET_Y] = (new Field("offset_y", -2))->setBounds(-50.0f, 50.0f);
    general[OFFSET_X] = (new Field("offset_x", -4))->setBounds(-50.0f, 50.0f);
    general[ELLIPSE_A] = (new Field("ellipse_A", 216))->setBounds(100.0f, general[WIDTH]->get<float>()/2);
    general[ELLIPSE_B] = (new Field("ellipse_B", 116))->setBounds(100.0f, general[HEIGHT]->get<float>()/2);
    general[PROMPT_WIDTH] = (new Field("prompt_width", 330))->setBounds(150.0f, general[WIDTH]->get<float>());
    general[PROMPT_HEIGHT] = (new Field("prompt_height", 120))->setBounds(150.0f, general[HEIGHT]->get<float>());

    general[NEEDLE_CENTER_RADIUS] = (new Field("needle_center_radius", 20))->setBounds(10.0f, 50.0f);
    general[NEEDLE_CENTER_OFFSET] = (new Field("needle_center_offset", 75))->setBounds(30.0f, 150.0f);
    general[NEEDLE_LENGTH_ADAPTIVE] = (new Field("needle_length_adaptive", 1))->setType(CHECKBOX);
    general[NEEDLE_LENGTH] = (new Field("needle_length", 100))->setBounds(0.0f, 150.0f);
    general[NEEDLE_TOP_WIDTH] = (new Field("needle_top_width", 2))->setBounds(0.0f, 30.0f);
    general[NEEDLE_BOTTOM_WIDTH] = (new Field("needle_bottom_width", 10))->setBounds(0.0f, 50.0f);

    general[TIME_POS_Y] = (new Field("time_pos_Y", -60))->setBounds(-150.0f, 150.0f);
    general[TIME_SIZE] = (new Field("time_size", 36, false))->setBounds(0.0f, 150.0f);
    general[DATE_POS_Y] = (new Field("date_pos_Y", -35))->setBounds(-150.0f, 150.0f);
    general[DATE_SIZE] = (new Field("date_size", 16, false))->setBounds(0.0f, 150.0f);
    general[SCALE_SIZE] = (new Field("scale_size", 12, false))->setBounds(0.0f, 150.0f);

    general[SCALE_LARGE_WIDTH] = (new Field("scale_large_width", 4))->setBounds(0.0f, 30.0f);
    general[SCALE_SMALL_WIDTH] = (new Field("scale_small_width", 2))->setBounds(0.0f, 30.0f);
    general[SCALE_LARGE_LENGTH] = (new Field("scale_large_length", 17))->setBounds(0.0f, 50.0f);
    general[SCALE_SMALL_LENGTH] = (new Field("scale_small_length", 7))->setBounds(0.0f, 50.0f);
    general[SCALE_LARGE_STEPS] = (new Field("scale_large_steps", 12))->setBounds(1.0f, 15.0f);
    general[SCALE_SMALL_STEPS] = (new Field("scale_small_steps", 4))->setBounds(1.0f, 15.0f);
    general[SCALE_ACC_COLOR_EVERY] = (new Field("scale_acc_color_every", 8))->setBounds(1.0f, 15.0f);
    general[SCALE_TEXT_STEPS] = (new Field("scale_text_steps", 4))->setBounds(1.0f, 15.0f);
    general[SCALE_TEXT_OFFSET] = (new Field("scale_text_offset", 35))->setBounds(0.0f, 100.0f);

    general[BACKGROUND_COLOR] = (new Field("background_color", (float)TFT_BLACK))->setType(COLOR);
    general[SCALE_COLOR] = (new Field("scale_color", TFT_GREEN))->setType(COLOR);
    general[SCALE_ACC_COLOR] = (new Field("scale_acc_color", TFT_RED))->setType(COLOR);
    general[NEEDLE_CENTER_COLOR] = (new Field("needle_center_color", 0x31A6))->setType(COLOR);
    general[FONT_COLOR] = (new Field("font_color", TFT_GREEN))->setType(COLOR);
    general[NEEDLE_COLOR] = (new Field("needle_color", TFT_RED))->setType(COLOR);


    for(int i=0; i<INPUT_SIZE; i++) {
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_R_OFFSET] = (new Field("r", 0.0f))->setStep(0.1f);
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_TYPE_OFFSET] = (new Field("type", Logarithmic))->setType(LIST);
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_BETA_OFFSET] = (new Field("beta", 0.0f))->setStep(0.1f);
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_R25_OFFSET] = (new Field("r25", 0.0f))->setStep(0.1f);
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_RMIN_OFFSET] = (new Field("r_min", 0.0f))->setStep(0.1f);
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_RMAX_OFFSET] = (new Field("r_max", 0.0f))->setStep(0.1f);
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_MAXVAL_OFFSET] = (new Field("max_val", 0.0f))->setStep(0.1f);
    }

    for(int i=ADS1115_0; i<=ADC_6; i++) {
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_NAME_OFFSET] = new Field("name", "");
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_UNIT_OFFSET] = new Field("unit", "");
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_SCALE_START_OFFSET] = new Field("scale_start", 0.0f);
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_SCALE_END_OFFSET] = new Field("scale_end", 0.0f);
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_PRECISION_OFFSET] = new Field("precision", 1);
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);
    }

    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_ENABLE_OFFSET] = (new Field("enable", 1))->setType(CHECKBOX);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_NAME_OFFSET] = new Field("name", "Voltage");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_UNIT_OFFSET] = new Field("unit", "V");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_SCALE_START_OFFSET] = new Field("scale_start", 6);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_SCALE_END_OFFSET] = new Field("scale_end", 18);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_PRECISION_OFFSET] = new Field("precision", 1);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_NAME_OFFSET] = new Field("name", "SW");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_UNIT_OFFSET] = new Field("unit", "°");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_SCALE_START_OFFSET] = new Field("scale_start", -550);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_SCALE_END_OFFSET] = new Field("scale_end", 550);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_PRECISION_OFFSET] = new Field("precision", 0.0f);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_STEERING_ANGLE + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_NAME_OFFSET] = new Field("name", "Speed");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_UNIT_OFFSET] = new Field("unit", "km/h");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_SCALE_START_OFFSET] = new Field("scale_start", 0.0f);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_SCALE_END_OFFSET] = new Field("scale_end", 280);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_PRECISION_OFFSET] = new Field("precision", 1);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_SPEED + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_RPM + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_RPM + DATA_NAME_OFFSET] = new Field("name", "RPM");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_RPM + DATA_UNIT_OFFSET] = new Field("unit", "rpm");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_RPM + DATA_SCALE_START_OFFSET] = new Field("scale_start", 6);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_RPM + DATA_SCALE_END_OFFSET] = new Field("scale_end", 8);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_RPM + DATA_PRECISION_OFFSET] = new Field("precision", 0.0f);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_RPM + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_GAS + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_GAS + DATA_NAME_OFFSET] = new Field("name", "Throttle");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_GAS + DATA_UNIT_OFFSET] = new Field("unit", "%");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_GAS + DATA_SCALE_START_OFFSET] = new Field("scale_start", 0.0f);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_GAS + DATA_SCALE_END_OFFSET] = new Field("scale_end", 100);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_GAS + DATA_PRECISION_OFFSET] = new Field("precision", 0.1f);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_GAS + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_HB + DATA_ENABLE_OFFSET] = (new Field("enable", 0.0f))->setType(CHECKBOX);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_HB + DATA_NAME_OFFSET] = new Field("name", "Handbrake");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_HB + DATA_UNIT_OFFSET] = new Field("unit", "");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_HB + DATA_SCALE_START_OFFSET] = new Field("scale_start", 0.0f);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_HB + DATA_SCALE_END_OFFSET] = new Field("scale_end", 1);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_HB + DATA_PRECISION_OFFSET] = new Field("precision", 0.0f);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * CAN_HB + DATA_VALUE_OFFSET] = new Field("value", 0.0f, false);

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
                            Log.logf("%d. %s: %s\n", i, general[i]->getName().c_str(), general[i]->getString().c_str());
#endif
                            break;
                        }
                        default: {
                            float f = doc[i].as<float>();
                            general[i]->set(f);
#ifdef LOG_SETTINGS
                            Log.logf("%d. %s: %f\n", i, general[i]->getName().c_str(), general[i]->get<float>());
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

void Settings::clear() {
    if(SPIFFS.exists("/settings.json"))
        SPIFFS.remove("/settings.json");
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
