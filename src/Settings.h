#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>
#include <fstream>
#include <cstring>
#include <TFT_eSPI.h>
#include "ArduinoJson.h"

#define OFFSET_X 0
#define OFFSET_Y 1

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
    Linear, Logarithmic, Dummy
};

const String inputTypeString[] = {"Linear", "Logarithmic"};

typedef struct InputSettings {
	boolean enable;
	char name[30];
	char unit[5];
	int16_t scaleStart;
	int16_t scaleEnd;
	float r;
	InputType type;
	float beta;
	float r25;
	float rmin;
	float rmax;
	float maxVal;
} InputSettings;

//typedef struct DataDisplaySettings {
//
//} DataDisplaySettings;

class Settings {

	protected:
		static Settings* settings;

	public:
		Settings();
		static Settings *getInstance();
		void loadDefault();
		void load();
		void save();

	 public:
		
		volatile InputSettings input[6];
		volatile VisualSettings visual;
//		volatile DataDisplaySettings leftGauge, rightGauge;

};

#endif
