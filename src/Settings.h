#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>
#include <fstream>
#include <cstring>
#include <TFT_eSPI.h>
#include "ArduinoJson.h"

#define OFFSET_X 0
#define OFFSET_Y 1


class Settings {

    protected:
        static Settings* settings;

    public:

        typedef struct VisualSettings {
           int16_t width;
           int16_t height;
           int16_t offsetX;
           int16_t offsetY;
           int16_t ellipseA;
           int16_t ellipseB;
           int16_t promptWidth;
           int16_t promptHeight;

           bool antialiasing;
           bool dark;
           bool drawMainScaleLine;

           int16_t needleCenterRadius;
           int16_t needleCenterOffset;
           int16_t needleLength;
           int16_t needleTopWidth; //in degrees
           int16_t needleBottomWidth; //in pixles

           int16_t timePosY;
           int16_t timeSize;
           int16_t datePosY;
           int16_t dateSize;
           int16_t scaleSize;

           int16_t scaleXLeft[5];
           int16_t scaleXRight[5];
           int16_t scaleY[5];

           int16_t scaleMainWidth;
           int16_t scaleLargeWidth;
           int16_t scaleSmallWidth;
           int16_t scaleLargeLength;
           int16_t scaleSmallLength;
           int16_t scaleLargeSteps;
           int16_t scaleSmallSteps;
           int16_t scaleAccColorEvery;
           int16_t scaleTextSteps;
           int16_t scaleAntialiasing;
           int16_t internalEllipseDistance;

           uint32_t backgroundColor;
           uint32_t scaleColor;
           uint32_t scaleAccColor;
           uint32_t needleCenterColor;
           uint32_t fontColor;
           uint32_t iconColor;
           uint32_t needleColor;
        } VisualSettings;

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
            ADS1115_0, ADS1115_1, ADS1115_2, ADS1115_3, ADC_5, ADC_6, VOLTAGE, CAN_RPM, LAST
        };

        const String dataSourceString[LAST] = {
                "ADS1115_0",
                "ADS1115_1",
                "ADS1115_2",
                "ADS1115_3",
                "ADC_5",
                "ADC_6",
                "VOLTAGE",
                "CAN_RPM"
        };

        //enum InputSource {
        //    ADS1115, ADC, CAN
        //};


        typedef struct DataDisplaySettings {
            boolean enable;
            char name[30];
            char unit[10];
            int16_t scaleStart;
            int16_t scaleEnd;
//            DataSource source;
            float value;
        } DataDisplaySettings;


	public:
		Settings();
		static Settings *getInstance();
		void loadDefault();
		void load();
		void save();
		void loadSelected(DataSource *selected);
		void saveSelected(DataSource *selected);

	 public:

		volatile InputSettings input[6];
		volatile DataDisplaySettings dataDisplay[LAST];
		volatile VisualSettings visual;

};

#endif
