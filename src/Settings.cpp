#include "Settings.h"

SettingsClass Settings;

void SettingsClass::loadDefault() {
    for(int i=0; i<SETTINGS_SIZE; i++)
        general[i]->setDefault();
}

void SettingsClass::init() {

    state = State();
    state.selected = new DataSource[3];

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

    general[VERSION] = new Field("version", "version", 1.0f, false);
    general[WIFI_SSID] = new Field("ssid", "SSID", MY_SSID);
    general[WIFI_PASS] = new Field("pass", "pass", MY_PASS);
    general[WIDTH] = new Field("width", "", 480, false);
    general[HEIGHT] = new Field("height", "", 320, false);
    general[OFFSET_Y] = (new Field("offset_y", "Y offset", -2))->setBounds(-50.0f, 50.0f);
    general[OFFSET_X] = (new Field("offset_x", "X offset", -4))->setBounds(-50.0f, 50.0f);
    general[ELLIPSE_A] = (new Field("ellipse_A", "Ellipse A", 216))->setBounds(100.0f, general[WIDTH]->get<float>()/2);
    general[ELLIPSE_B] = (new Field("ellipse_B", "Ellipse B", 116))->setBounds(100.0f, general[HEIGHT]->get<float>()/2);
    general[PROMPT_WIDTH] = (new Field("prompt_width", "Prompt width", 330))->setBounds(150.0f, general[WIDTH]->get<float>());
    general[PROMPT_HEIGHT] = (new Field("prompt_height", "Prompt height", 120))->setBounds(150.0f, general[HEIGHT]->get<float>());

    general[NEEDLE_CENTER_RADIUS] = (new Field("needle_center_radius", "Needle center radius", 20))->setBounds(10.0f, 50.0f);
    general[NEEDLE_CENTER_OFFSET] = (new Field("needle_center_offset", "Needle center offset", 75))->setBounds(30.0f, 150.0f);
    general[NEEDLE_LENGTH_ADAPTIVE] = (new Field("needle_length_adaptive", "Adaptive needle negth", 1))->setType(CHECKBOX);
    general[NEEDLE_LENGTH] = (new Field("needle_length", "Needle length", 100))->setBounds(0.0f, 150.0f);
    general[NEEDLE_TOP_WIDTH] = (new Field("needle_top_width", "Needle top width:", 2))->setBounds(0.0f, 30.0f);
    general[NEEDLE_BOTTOM_WIDTH] = (new Field("needle_bottom_width", "Needle bottom width", 10))->setBounds(0.0f, 50.0f);

    general[TIME_POS_Y] = (new Field("time_pos_Y", "Time position Y", -60))->setBounds(-150.0f, 150.0f);
//    general[TIME_SIZE] = (new Field("time_size", 36, false))->setBounds(0.0f, 150.0f);
    general[DATE_POS_Y] = (new Field("date_pos_Y", "Date position Y", -35))->setBounds(-150.0f, 150.0f);
//    general[DATE_SIZE] = (new Field("date_size", 16, false))->setBounds(0.0f, 150.0f);
//    general[SCALE_SIZE] = (new Field("scale_size", 12, false))->setBounds(0.0f, 150.0f);

    general[SCALE_LARGE_WIDTH] = (new Field("scale_large_width", "scale_large_width", 4))->setBounds(0.0f, 30.0f);
    general[SCALE_SMALL_WIDTH] = (new Field("scale_small_width", "scale_small_width", 2))->setBounds(0.0f, 30.0f);
    general[SCALE_LARGE_LENGTH] = (new Field("scale_large_length", "scale_large_length", 17))->setBounds(0.0f, 50.0f);
    general[SCALE_SMALL_LENGTH] = (new Field("scale_small_length", "scale_small_length", 7))->setBounds(0.0f, 50.0f);
    general[SCALE_LARGE_STEPS] = (new Field("scale_large_steps", "scale_large_steps", 12))->setBounds(1.0f, 15.0f);
    general[SCALE_SMALL_STEPS] = (new Field("scale_small_steps", "scale_small_steps", 4))->setBounds(1.0f, 15.0f);
    general[SCALE_ACC_COLOR_EVERY] = (new Field("scale_acc_color_every", "scale_acc_color_every", 8))->setBounds(1.0f, 15.0f);
    general[SCALE_TEXT_STEPS] = (new Field("scale_text_steps", "scale_text_steps", 4))->setBounds(1.0f, 15.0f);
    general[SCALE_TEXT_OFFSET] = (new Field("scale_text_offset", "scale_text_offset", 35))->setBounds(0.0f, 100.0f);

    general[BACKGROUND_COLOR] = (new Field("background_color", "background_color", (float)TFT_BLACK))->setType(COLOR);
    general[SCALE_COLOR] = (new Field("scale_color", "scale_color", TFT_GREEN))->setType(COLOR);
    general[SCALE_ACC_COLOR] = (new Field("scale_acc_color", "scale_acc_color", TFT_RED))->setType(COLOR);
    general[NEEDLE_CENTER_COLOR] = (new Field("needle_center_color", "needle_center_color", 0x31A6))->setType(COLOR);
    general[FONT_COLOR] = (new Field("font_color", "font_color", TFT_GREEN))->setType(COLOR);
    general[NEEDLE_COLOR] = (new Field("needle_color", "needle_color", TFT_RED))->setType(COLOR);


    for(int i=0; i<INPUT_SIZE; i++) {
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_PULLUP_OFFSET] = (new Field("pullup_" + (String)i + "_" + INPUT_PULLUP_OFFSET, "Pull-up R", 0.0f))->setStep(0.1f);
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_PULLDOWN_OFFSET] = (new Field("pulldown_" + (String)i + "_" + INPUT_PULLDOWN_OFFSET, "Pull-down R", 0.0f, false))->setStep(0.1f);
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_SERIES_OFFSET] = (new Field("input_" + (String)i + "_" + INPUT_SERIES_OFFSET, "Input R", 0.0f, false))->setStep(0.1f);
        general[INPUT_BEGIN_BEGIN + INPUT_SETTINGS_SIZE * i + INPUT_EXPRESSION_OFFSET] = new Field("exp_" + (String)i + "_" + INPUT_EXPRESSION_OFFSET, "Wyrażenie", i==(INPUT_SIZE-1) ? "v*5.7" : "v");
    }

    for(int i=ADS1115_0; i<=VOLTAGE; i++) {
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_ENABLE_OFFSET] = (new Field("enable_" + (String)i + "_" + DATA_ENABLE_OFFSET, "włączone", 0.0f))->setType(CHECKBOX);
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_NAME_OFFSET] = new Field("name_" + (String)i + "_" + DATA_NAME_OFFSET, "nazwa", "");
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_UNIT_OFFSET] = new Field("unit_" + (String)i + "_" + DATA_UNIT_OFFSET, "jednostka", "");
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_SCALE_START_OFFSET] = new Field("scale_start_" + (String)i + "_" + DATA_SCALE_START_OFFSET, "początek skali", 0.0f);
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_SCALE_END_OFFSET] = new Field("scale_end_" + (String)i + "_" + DATA_SCALE_END_OFFSET, "koniec skali", 0.0f);
        general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * i + DATA_PRECISION_OFFSET] = new Field("precision_" + (String)i + "_" + DATA_PRECISION_OFFSET, "precyzja", 1);
    }

    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_ENABLE_OFFSET] = (new Field("voltage_enable", "włączone", 1))->setType(CHECKBOX);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_NAME_OFFSET] = new Field("voltage_name", "nazwa", "Voltage");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_UNIT_OFFSET] = new Field("voltage_unit", "jednostka", "V");
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_SCALE_START_OFFSET] = new Field("voltage_scale_start", "początek skaliv", 6);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_SCALE_END_OFFSET] = new Field("voltage_scale_end", "koniec skali", 18);
    general[DATA_BEGIN_BEGIN + DATA_SETTINGS_SIZE * VOLTAGE + DATA_PRECISION_OFFSET] = new Field("voltage_precision", "precyzja", 1);

}

S_STATUS SettingsClass::load() {

    S_STATUS status = S_SUCCESS;

    Log.logf("Loading settings");
    if(SPIFFS.exists("/settings.json")) {
        JsonDocument doc;
        fs::File file = SPIFFS.open("/settings.json", "r");
        DeserializationError error = deserializeJson(doc, file);
        if (error) {
            Log.logf("Failed to read file, using default");
            status = S_FAIL;
        } else {
            int v = doc[general[VERSION]->getId()] | 0;
            general[VERSION]->setDefault();
            if(v != general[VERSION]->get<int>()) {
                Log.logf("Setting version has changed. Using default");
                status = S_FAIL;
            } else {
                for(int i=1; i<SETTINGS_SIZE; i++) {
                    if(general[i]->isConfigurable()) {
                        if(doc[general[i]->getId()].isNull()) {
                            Log.logf("Missing: %d: %s\n", i, general[i]->getId().c_str());
                            status = S_MISSING;
                        } else {
                            switch (general[i]->getType()) {
                                case STRING: {
                                    char c[64] = "";
                                    strcpy((char *) c, doc[general[i]->getId().c_str()]);
                                    general[i]->set((const char *) c);
#ifdef LOG_SETTINGS
                                    Log.logf("%d. %s %s %s\n", i, general[i]->getId().c_str(), general[i]->getName().c_str(),
                                             general[i]->getString().c_str());
#endif
                                    break;
                                }
                                default: {
                                    float f = doc[general[i]->getId().c_str()].as<float>();
                                    general[i]->set(f);
#ifdef LOG_SETTINGS
                                    Log.logf("%d. %s %s: %f\n", i, general[i]->getId().c_str(), general[i]->getName().c_str(),
                                             general[i]->get<float>());
#endif
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        file.close();

    } else {
        Log.logf("File not found, using defaults");
        status = S_FAIL;
    }
    return status;
}

S_STATUS SettingsClass::save(bool waitForCompletion) {
    TaskHandle_t handle;
    S_STATUS status = S_PENDING;
    if(!xTaskCreate([](void * status) {
            Log.logf("Saving settings");
            if(SPIFFS.exists("/settings.json"))
                SPIFFS.remove("/settings.json");
            fs::File file = SPIFFS.open("/settings.json", "w");
            JsonDocument doc;
            doc[Settings.general[VERSION]->getId()] = Settings.general[VERSION]->get<float>();
            for(int i=1; i<SETTINGS_SIZE; i++) {
                if(Settings.general[i]->isConfigurable()) {
                    switch (Settings.general[i]->getType()) {
                        case STRING: {
                            doc[Settings.general[i]->getId()] = (String)Settings.general[i]->getString().c_str();
#ifdef LOG_SETTINGS
                            Log.logf("%s: %s %s\n", Settings.general[i]->getId().c_str(), Settings.general[i]->getName().c_str(), Settings.general[i]->getString().c_str());
#endif
                            break;
                        }
                        default: {
                            doc[Settings.general[i]->getId()] = Settings.general[i]->get<float>();
#ifdef LOG_SETTINGS
                            Log.logf("%s: %s %f\n", Settings.general[i]->getId().c_str(), Settings.general[i]->getName().c_str(), Settings.general[i]->get<float>());
#endif
                            break;
                        }
                    }
                }
            }

            if (serializeJson(doc, file) == 0) {
                Log.logf("Failed to write to file");
                *(S_STATUS*)status = S_FAIL;
            } else {
                Log.logf("Settings saved");
                *(S_STATUS*)status = S_SUCCESS;
            }
            file.close();
            vTaskDelete(nullptr);
        },
        "saveSettings",
        8*1024,
        &status,
        1,
        &handle))
        Log.logf("Failed to start saveSettings task");

    if(waitForCompletion)
        while(status == S_PENDING)
            delay(1);
    return status;
}

void SettingsClass::clear() {
    if(SPIFFS.exists("/settings.json"))
        SPIFFS.remove("/settings.json");
}

void SettingsClass::loadState() {
    Log.logf("Loading state");
    if(SPIFFS.exists("/state.json")) {
        JsonDocument doc;
        fs::File file = SPIFFS.open("/state.json", "r");
        DeserializationError error = deserializeJson(doc, file);
        state.selected[0] = doc["sel_0"] | VOLTAGE;
        state.selected[1] = doc["sel_1"] | VOLTAGE;
        state.selected[2] = doc["sel_2"] | VOLTAGE;
        if (error)
            Log.logf("Failed to read file, using default state");
        else
            Log.logf("State loaded");
    } else {
        state.selected[0] = VOLTAGE;
        state.selected[1] = VOLTAGE;
        state.selected[2] = VOLTAGE;
        Log.logf("File not found, using default state");
    }
}

void SettingsClass::saveState() {
    if(SPIFFS.exists("/state.json"))
        SPIFFS.remove("/state.json");
    fs::File file = SPIFFS.open("/state.json", "w");
    JsonDocument doc;

    doc["sel_0"] = state.selected[0];
    doc["sel_1"] = state.selected[1];
    doc["sel_2"] = state.selected[2];

    if (serializeJson(doc, file) == 0)
        Log.logf("Failed to write to file");

    Log.logf("State saved");

    file.close();
}
