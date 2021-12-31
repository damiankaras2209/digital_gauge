#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include "Log.h"

#include <stdint.h>
#include <fstream>
#include <cstring>
#include <TFT_eSPI.h>
#include "ArduinoJson.h"

enum Fields : unsigned int {
    SSID,
    PASS,

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
    NEEDLE_LENGTH,
    NEEDLE_TOP_WIDTH, //in degrees
    NEEDLE_BOTTOM_WIDTH, //in pixles

    TIME_POS_Y,
    TIME_SIZE,
    DATE_POS_Y,
    DATE_SIZE,
    SCALE_SIZE,

    SCALE_MAIN_WIDTH,
    SCALE_LARGE_WIDTH,
    SCALE_SMALL_WIDTH,
    SCALE_LARGE_LENGTH,
    SCALE_SMALL_LENGTH,
    SCALE_LARGE_STEPS,
    SCALE_SMALL_STEPS,
    SCALE_ACC_COLOR_EVERY,
    SCALE_TEXT_STEPS,
    SCALE_TEXT_OFFSET,
//    SCALE_ANTIALIASING,
//    INTERNAL_ELLIPSE_DISTANCE,

    BACKGROUND_COLOR,
    SCALE_COLOR,
    SCALE_ACC_COLOR,
    NEEDLE_CENTER_COLOR,
    FONT_COLOR,
    ICON_COLOR,
    NEEDLE_COLOR,
    FIELDS_SIZE

};

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

enum Type {
    FLOAT, STRING//, INPUTS, DATA
};

struct Field {
    Type _type;
    bool _configurable;
    std::string _name;
    float _defaultFloat;
    volatile float _valueFloat;
    std::string _defaultChar;
    std::string _valueChar;
    Field(const char* c, float d, bool configurable = true) {
        _type = FLOAT;
        _configurable = configurable;
        _name = std::string(c);
        _valueFloat = d;
        _defaultFloat = d;
    }

    Field(const char* c, const char* d, bool configurable = true) {
        _type = STRING;
        _configurable = configurable;
        _name = std::string(c);
        _defaultChar = std::string(d);
        _valueChar = std::string(d);
    }

    void set(float v) {
        _valueFloat = v;
    }

    void set(const char* v) {
        _valueChar = std::string(v);
    }

    void setDefault() {
        _valueFloat = _defaultFloat;
        _valueChar = std::string(_defaultChar);
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
        return _valueChar;
    }

    std::string getDefaultString() {
        return _defaultChar;
    }

    std::string getChar() {
        return _valueChar.c_str();
    }

    Type getType() const {
        return _type;
    }
};


class Settings {

    protected:
        static Settings* settings;

    public:

        enum InputType {
            Linear, Logarithmic, Voltage, Dummy
        };

        const String inputTypeString[Dummy] = {"Linear", "Logarithmic", "Voltage"};

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

        Field* general[Fields::FIELDS_SIZE];

		volatile InputSettings input[6];
		volatile DataDisplaySettings dataDisplay[LAST];

};

#endif
