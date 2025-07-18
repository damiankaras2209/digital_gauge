#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <sstream>
#include <cstdint>
#include <fstream>
#include <cstring>
#include <TFT_eSPI.h>
#include <iomanip>

#include "ArduinoJson.h"

#include "Log.h"
#include "Networking/Networking.h"

enum S_STATUS {
    S_PENDING, S_SUCCESS, S_FAIL, S_MISSING
};

enum Field : uint16_t{

    VERSION,

    DEMO,
    WIFI_SSID,
    WIFI_PASS,

    WIDTH,
    HEIGHT,
    OFFSET_Y,
    OFFSET_X,
    ELLIPSE_A, //half of ellipse width (x-axis)
    ELLIPSE_B, //half of ellipse height (y-axis)
    PROMPT_WIDTH,
    PROMPT_HEIGHT,

    NEEDLE_CENTER_RADIUS,
    NEEDLE_CENTER_OFFSET,
    NEEDLE_LENGTH_ADAPTIVE,
    NEEDLE_LENGTH,
    NEEDLE_TOP_WIDTH, //in degrees
    NEEDLE_BOTTOM_WIDTH,

    TIME_POS_Y,
//    TIME_SIZE,
    DATE_POS_Y,
//    DATE_SIZE,
//    SCALE_SIZE,
    STATUS_POS_X,
    STATUS_POS_Y,

    SCALE_LARGE_WIDTH,
    SCALE_SMALL_WIDTH,
    SCALE_LARGE_LENGTH,
    SCALE_SMALL_LENGTH,
    SCALE_LARGE_STEPS,
    SCALE_SMALL_STEPS,
    SCALE_ACC_COLOR_EVERY,
    SCALE_TEXT_STEPS,
    SCALE_TEXT_OFFSET,

    BACKGROUND_COLOR,
    SCALE_COLOR,
    SCALE_ACC_COLOR,
    NEEDLE_CENTER_COLOR,
    FONT_COLOR,
    NEEDLE_COLOR,

    INPUT_BEGIN_BEGIN, INPUT_0_1,  INPUT_0_2,  INPUT_0_END,
    INPUT_1_BEGIN,     INPUT_1_1,  INPUT_1_2,  INPUT_1_END,
    INPUT_2_BEGIN,     INPUT_2_1,  INPUT_2_2,  INPUT_2_END,
    INPUT_3_BEGIN,     INPUT_3_1,  INPUT_3_2,  INPUT_3_END,
    INPUT_4_BEGIN,     INPUT_4_1,  INPUT_4_2,  INPUT_4_END,
    INPUT_5_BEGIN,     INPUT_5_1,  INPUT_5_2,  INPUT_5_END,
    INPUT_6_BEGIN,     INPUT_6_1,  INPUT_6_2,  INPUT_END_END,

    DATA_BEGIN_BEGIN,  DATA_0_1,   DATA_0_2,   DATA_0_3,   DATA_0_4,   DATA_0_END,
    DATA_1_BEGIN,      DATA_1_1,   DATA_1_2,   DATA_1_3,   DATA_1_4,   DATA_1_END,
    DATA_2_BEGIN,      DATA_2_1,   DATA_2_2,   DATA_2_3,   DATA_2_4,   DATA_2_END,
    DATA_3_BEGIN,      DATA_3_1,   DATA_3_2,   DATA_3_3,   DATA_3_4,   DATA_3_END,
    DATA_4_BEGIN,      DATA_4_1,   DATA_4_2,   DATA_4_3,   DATA_4_4,   DATA_4_END,
    DATA_5_BEGIN,      DATA_5_1,   DATA_5_2,   DATA_5_3,   DATA_5_4,   DATA_5_END,
    DATA_6_BEGIN,      DATA_6_1,   DATA_6_2,   DATA_6_3,   DATA_6_4,   DATA_6_END,
    DATA_7_BEGIN,      DATA_7_1,   DATA_7_2,   DATA_7_3,   DATA_7_4,   DATA_7_END,
    DATA_8_BEGIN,      DATA_8_1,   DATA_8_2,   DATA_8_3,   DATA_8_4,   DATA_8_END,
    DATA_9_BEGIN,      DATA_9_1,   DATA_9_2,   DATA_9_3,   DATA_9_4,   DATA_9_END,
    DATA_10_BEGIN,     DATA_10_1,  DATA_10_2,  DATA_10_3,  DATA_10_4,  DATA_10_END,
    DATA_11_BEGIN,     DATA_11_1,  DATA_11_2,  DATA_11_3,  DATA_11_4,  DATA_END_END,

    SETTINGS_LAST
};

#define GENERAL_SETTINGS_SIZE INPUT_BEGIN_BEGIN

#define VISUAL_SETTINGS_START WIDTH
#define VISUAL_SETTINGS_END (INPUT_BEGIN_BEGIN - 1)

#define INPUT_PULLUP_OFFSET 0
#define INPUT_PULLDOWN_OFFSET 1
#define INPUT_SERIES_OFFSET 2
#define INPUT_EXPRESSION_OFFSET 3

#define INPUT_SETTINGS_SIZE (INPUT_0_END - INPUT_BEGIN_BEGIN + 1) //input fields count
#define INPUT_SIZE (INPUT_END_END - INPUT_BEGIN_BEGIN + 1)/INPUT_SETTINGS_SIZE //number of inputs

#define DATA_ENABLE_OFFSET 0
#define DATA_NAME_OFFSET 1
#define DATA_UNIT_OFFSET 2
#define DATA_SCALE_START_OFFSET 3
#define DATA_SCALE_END_OFFSET 4
#define DATA_PRECISION_OFFSET 5

#define DATA_SETTINGS_SIZE (DATA_0_END - DATA_BEGIN_BEGIN + 1) //data fields count
#define DATA_SIZE (DATA_END_END - DATA_BEGIN_BEGIN + 1)/DATA_SETTINGS_SIZE //number of data

#define SETTINGS_SIZE SETTINGS_LAST //all settings count


typedef struct DataDisplaySettings {
    boolean enable = false;
    char name[30] = "";
    char unit[10] = "";
    int16_t scaleStart = 0;
    int16_t scaleEnd = 100;
    //            DataSource source;
    float value = 0;
} DataDisplaySettings;

class SettingsClass {

    public:

        typedef struct InputSettings {
            float r;
            float beta;
            float r25;
            float rmin;
            float rmax;
            float maxVal;
        } InputSettings;

        enum DataSource {
            ADS1115_0, ADS1115_1, ADS1115_2, ADS1115_3, ADC_5, ADC_6, VOLTAGE,
            CAN_STEERING_ANGLE, CAN_RPM, CAN_SPEED, CAN_GAS, CAN_HB, LAST
        };

        const String dataSourceString[LAST] = {
                "ADS1115_0",
                "ADS1115_1",
                "ADS1115_2",
                "ADS1115_3",
                "ADC_4",
                "ADC_5",
                "VOLTAGE",
                "CAN_STEERING_ANGLE",
                "CAN_RPM",
                "CAN_SPEED",
                "CAN_GAS",
                "CAN_HB"
        };

        //enum InputSource {
        //    ADS1115, ADC, CAN
        //};


        typedef struct DataDisplaySettings {
            boolean enable = false;
            char name[30] = "";
            char unit[10] = "";
            int16_t scaleStart = 0;
            int16_t scaleEnd = 100;
//            DataSource source;
            float value = 0;
        } DataDisplaySettings;


        enum Type {
            FLOAT, STRING, LIST, CHECKBOX, COLOR
        };

        struct Field {
            Type _type;
            bool _configurable;
            std::string _id;
            std::string _name;

            volatile float _valueFloat;
            float _defaultFloat;
            float _step;
            float _min;
            float _max;

            std::string _valueString;
            std::string _defaultString;

            Field(String id, String name, float defaultValue, bool configurable = true) {
                _type = FLOAT;
                _configurable = configurable;
                _id = std::string(id.c_str());
                _name = std::string(name.c_str());
                _valueFloat = defaultValue;
                _defaultFloat = defaultValue;
                _step = 1.0f;
                _min = 0.0f;
                _max = 0.0f;
            }

            Field(String id, String name, std::string defaultString, bool configurable = true) {
                _type = STRING;
                _configurable = configurable;
                _id = std::string(id.c_str());
                _name = std::string(name.c_str());
                _defaultString = std::string(defaultString);
                _valueString = std::string(defaultString);
            }

            void set(float v) {
                _valueFloat = v;
            }

            void set(const char* v) {
                _valueString = std::string(v);
            }

            Field* setType(Type t) {
                _type = t;
                return this;
            }

            Field* setStep(float s) {
                _step = s;
                return this;
            }

            Field* setBounds(float min, float max) {
                _min = min;
                _max = max;
                return this;
            }

            void setDefault() {
                _valueFloat = _defaultFloat;
                _valueString = std::string(_defaultString);
            }

            std::string getId() {
                return _id;
            }

            std::string getName() {
                return _name;
            }

            bool isConfigurable() {
                return _configurable;
            }

            template<class T>
                    T get() {
                return static_cast<T>(_valueFloat);
            }

            template<class T>
                    T getDefault() {
                return static_cast<T>(_defaultFloat);
            }

            std::string getString() {
                return _valueString;
            }

            std::string getDefaultString() {
                return _defaultString;
            }

            Type getType() const {
                return _type;
            }

            std::string getHTMLInput(int id) {
                std::stringstream ss;
                ss << "<div>";
                ss << "<label for='" << id << "'>" << _name << "</label>";
                ss << "<input class='" << _name << "'";
                switch (_type) {
                    case FLOAT: {
                        if(_min != _max) {
                            ss << " value='" << _valueFloat << "'";
                            ss << " type='range'";
                            ss << " step='" << _step << "'";
                            ss << " min='" << _min << "' max='" << _max << "'";
                            ss << " id='" << id << "'";
                            ss << " oninput='this.nextElementSibling.value = this.value'>";
                            ss << "<output>" << _valueFloat << "</output>";
                        } else {
                            ss << " value='" << _valueFloat <<"'";
                            ss << " type='number'";
                            ss << " step='" << _step << "'";
                            ss << " id='" << id << "'";
                            ss << "<";
                        }
                        break;
                    }
                    case STRING: {
                        ss << " value='" << _valueString << "'";
                        ss << " type='text'";
                        ss << " id='" << id << "'";
                        ss << ">";
                        break;
                    }
//                    case LIST: {
//                        ss << "<select id='";
//                        ss << id;
//                        ss << "'>";
//                        for(int i=0; i<InputType::InputTypeSize; i++) {
//                            ss << "<option value='" << i <<"' " << ((int)_valueFloat == i ? "selected" : "") << ">" << inputTypeString[i].c_str() << "</option>";
//                        }
//                        ss << "</select>";
//                        break;
//                    }
                    case CHECKBOX: {
                        ss << " type='checkbox' id='" << id << "'";
                        ss << (_valueFloat == 1 ? " checked" : "");
                        ss << ">";
                        break;
                    }
                    case COLOR: {
                        uint8_t r = ((int)_valueFloat >> 8) & 0xF8; r |= (r >> 5);
                        uint8_t g = ((int)_valueFloat >> 3) & 0xFC; g |= (g >> 6);
                        uint8_t b = ((int)_valueFloat << 3) & 0xF8; b |= (b >> 5);
                        ss << " value='#" << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << (((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0)) << "'";
                        ss << " type='color'";
                        ss << " id='" << std::dec << id << "'";
                        ss << ">";
                        break;
                    }
                }
                ss << "</div>";
                return ss.str();
            }
        };

    typedef struct State {
        DataSource* selected;
        bool throttleState;
        NetworkingClass::NetworkType networkType;
    } State;


	public:
		void init();
		void loadDefault();
		S_STATUS load();
        S_STATUS save(bool waitForCompletion = true);
		void clear();
		void loadState();
		void saveState();

	public:
        Field* general[SETTINGS_SIZE];
        State state;

};

extern SettingsClass Settings;

#endif
