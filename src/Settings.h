#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "Log.h"

#include <stdint.h>
#include <fstream>
#include <cstring>
#include <TFT_eSPI.h>
#include <iomanip>
#include "ArduinoJson.h"

#define WIFI_SSID 0
#define WIFI_PASS 1

#define WIDTH 2
#define HEIGHT 3
#define OFFSET_Y 4
#define OFFSET_X 5
#define ELLIPSE_A 6//half of ellipse width (x-axis)
#define ELLIPSE_B 7//half of ellipse height (y-axis)
#define PROMPT_WIDTH 8
#define PROMPT_HEIGHT 9

#define NEEDLE_CENTER_RADIUS 10
#define NEEDLE_CENTER_OFFSET 11
#define NEEDLE_LENGTH 12
#define NEEDLE_TOP_WIDTH 13//in degrees
#define NEEDLE_BOTTOM_WIDTH 14

#define TIME_POS_Y 15
#define TIME_SIZE 16
#define DATE_POS_Y 17
#define DATE_SIZE 18

#define SCALE_SIZE 19
#define SCALE_MAIN_WIDTH 20
#define SCALE_LARGE_WIDTH 21
#define SCALE_SMALL_WIDTH 22
#define SCALE_LARGE_LENGTH 23
#define SCALE_SMALL_LENGTH 24
#define SCALE_LARGE_STEPS 25
#define SCALE_SMALL_STEPS 26
#define SCALE_ACC_COLOR_EVERY 27
#define SCALE_TEXT_STEPS 28
#define SCALE_TEXT_OFFSET 29
//#define    //    SCALE_ANTIALIASING,
//#define    //    INTERNAL_ELLIPSE_DISTANCE,

#define BACKGROUND_COLOR 30
#define SCALE_COLOR 31
#define SCALE_ACC_COLOR 32
#define NEEDLE_CENTER_COLOR 33
#define FONT_COLOR 34
#define ICON_COLOR 35
#define NEEDLE_COLOR 36

#define GENERAL_SETTINGS_SIZE 37

#define INPUT_R_OFFSET 0
#define INPUT_TYPE_OFFSET 1
#define INPUT_BETA_OFFSET 2
#define INPUT_R25_OFFSET 3
#define INPUT_RMIN_OFFSET 4
#define INPUT_RMAX_OFFSET 5
#define INPUT_MAXVAL_OFFSET 6

#define INPUT_SETTINGS_SIZE 7 //input fields count

#define INPUT_0 GENERAL_SETTINGS_SIZE
#define INPUT_1 INPUT_0 + INPUT_SETTINGS_SIZE * 1
#define INPUT_2 INPUT_0 + INPUT_SETTINGS_SIZE * 2
#define INPUT_3 INPUT_0 + INPUT_SETTINGS_SIZE * 3
#define INPUT_4 INPUT_0 + INPUT_SETTINGS_SIZE * 4
#define INPUT_5 INPUT_0 + INPUT_SETTINGS_SIZE * 5

#define INPUT_SIZE 6 //number of inputs

#define DATA_ENABLE_OFFSET 0
#define DATA_NAME_OFFSET 1
#define DATA_UNIT_OFFSET 2
#define DATA_SCALE_START_OFFSET 3
#define DATA_SCALE_END_OFFSET 4
#define DATA_PRECISION_OFFSET 5
#define DATA_VALUE_OFFSET 6

#define DATA_SETTINGS_SIZE 7 //data fields count

#define DATA_0 INPUT_0 + INPUT_SIZE * INPUT_SETTINGS_SIZE

#define DATA_SIZE 12

#define SETTINGS_SIZE DATA_0 + DATA_SIZE * DATA_SETTINGS_SIZE //all settings count


typedef struct DataDisplaySettings {
    boolean enable = false;
    char name[30] = "";
    char unit[10] = "";
    int16_t scaleStart = 0;
    int16_t scaleEnd = 100;
    //            DataSource source;
    float value = 0;
} DataDisplaySettings;

//enum Fields : unsigned int {
//    SSID,
//    PASS,
//
//    WIDTH,
//    HEIGHT,
//    OFFSET_Y,
//    OFFSET_X,
//    ELLIPSE_A, //half of ellipse width (x-axis)
//    ELLIPSE_B, //half of ellipse height (y-axis)
//    PROMPT_WIDTH,
//    PROMPT_HEIGHT,
//
//    NEEDLE_CENTER_RADIUS,
//    NEEDLE_CENTER_OFFSET,
//    NEEDLE_LENGTH,
//    NEEDLE_TOP_WIDTH, //in degrees
//    NEEDLE_BOTTOM_WIDTH, //in pixles
//
//    TIME_POS_Y,
//    TIME_SIZE,
//    DATE_POS_Y,
//    DATE_SIZE,
//    SCALE_SIZE,
//
//    SCALE_MAIN_WIDTH,
//    SCALE_LARGE_WIDTH,
//    SCALE_SMALL_WIDTH,
//    SCALE_LARGE_LENGTH,
//    SCALE_SMALL_LENGTH,
//    SCALE_LARGE_STEPS,
//    SCALE_SMALL_STEPS,
//    SCALE_ACC_COLOR_EVERY,
//    SCALE_TEXT_STEPS,
//    SCALE_TEXT_OFFSET,
////    SCALE_ANTIALIASING,
////    INTERNAL_ELLIPSE_DISTANCE,
//
//    BACKGROUND_COLOR,
//    SCALE_COLOR,
//    SCALE_ACC_COLOR,
//    NEEDLE_CENTER_COLOR,
//    FONT_COLOR,
//    ICON_COLOR,
//    NEEDLE_COLOR,
//    FIELDS_SIZE
//
//};

//enum InputFields {
//    R,
//    TYPE,
//    BETA,
//    R25,
//    RMIN,
//    RMAX,
//    MAX_VAL,
//    INPUT_FIELDS_SIZE
//};



class Settings {

    protected:
        static Settings* settings;

    public:

        enum InputType {
            Linear, Logarithmic, Voltage, InputTypeSize
        };

        const String inputTypeString[InputTypeSize] = {"Linear", "Logarithmic", "Voltage"};

        typedef struct InputSettings {
            float r;
            InputType type;
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
                "ADC_5",
                "ADC_6",
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
            std::string _name;

            volatile float _valueFloat;
            float _defaultFloat;
            float _step;

            std::string _valueString;
            std::string _defaultString;

            Field(const char* c, float d, bool configurable = true) {
                _type = FLOAT;
                _configurable = configurable;
                _name = std::string(c);
                _valueFloat = d;
                _defaultFloat = d;
                _step = 1.0f;
            }

            Field(const char* c, const char* d, bool configurable = true) {
                _type = STRING;
                _configurable = configurable;
                _name = std::string(c);
                _defaultString = std::string(d);
                _valueString = std::string(d);
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

            void setDefault() {
                _valueFloat = _defaultFloat;
                _valueString = std::string(_defaultString);
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
                switch (_type) {
                    case FLOAT: {
                        ss << "<input value='";
                        ss << _valueFloat;
                        ss << "' type='number' step='";
                        ss << _step;
                        ss << "' id='";
                        ss << id;
                        ss << "'>";
                        break;
                    }
                    case STRING: {
                        ss << "<input value='";
                        ss << _valueString;
                        ss << "' type='text'";
                        ss << " id='";
                        ss << id;
                        ss << "'>";
                        break;
                    }
                    case LIST: {
                        ss << "<select id='";
                        ss << id;
                        ss << "'>";
                        for(int i=0; i<InputType::InputTypeSize; i++) {
                            ss << "<option value='" << i <<"' " << ((int)_valueFloat == i ? "selected" : "") << ">" << settings->inputTypeString[i].c_str() << "</option>";
                        }
                        ss << "</select>";
                        break;
                    }
                    case CHECKBOX: {
                        ss << "<input type='checkbox' id='";
                        ss << id;
                        ss << "' ";
                        ss << (_valueFloat == 1 ? "checked" : "");
                        ss << ">";
                        break;
                    }
                    case COLOR: {
                        uint8_t r = ((int)_valueFloat >> 8) & 0xF8; r |= (r >> 5);
                        uint8_t g = ((int)_valueFloat >> 3) & 0xFC; g |= (g >> 6);
                        uint8_t b = ((int)_valueFloat << 3) & 0xF8; b |= (b >> 5);
                        ss << "<input value='#";
                        ss << std::hex << std::uppercase << std::setfill('0') << std::setw(6) << (((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0));
                        ss << "' type='color' id='";
                        ss << std::dec << id;
                        ss << "'>";
                        break;
                    }
                }
                return ss.str();
            }
        };


	public:
		Settings();
		static Settings *getInstance();
		void init();
		void loadDefault();
		void load();
		void save();
		void loadSelected(DataSource *selected);
		void saveSelected(DataSource *selected);

	 public:

        Field* general[SETTINGS_SIZE];

};

#endif
