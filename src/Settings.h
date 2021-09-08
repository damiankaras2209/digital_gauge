#ifndef _SETTINGS_H_
#define _SETTINGS_H_

#include <stdint.h>
#include <fstream>
#include <TFT_eSPI.h>
#include "ArduinoJson.h"

#define OFFSET_X 0
#define OFFSET_Y 1

typedef struct InputData {
	boolean enable;
	float r;
	int type;
	float beta;
	float r25;
	float rmin;
	float rmax;
	float maxVal;
} InputData;

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
		volatile int16_t width;
		volatile int16_t height;
		volatile int16_t offsetX;
		volatile int16_t offsetY;
		volatile int16_t ellipseA;
		volatile int16_t ellipseB;

		volatile bool antialiasing;
		volatile bool dark;
		volatile bool drawMainScaleLine;

		volatile int16_t needleCenterRadius;
		volatile int16_t needleCenterOffset;
		volatile int16_t needleLength;
		volatile int16_t needleTopWidth; //in degrees
		volatile int16_t needleBottomWidth; //in pixles

		volatile int16_t timePosY;
		volatile int16_t timeSize;
		volatile int16_t datePosY;
		volatile int16_t dateSize;
		volatile int16_t scaleSize;

		volatile int16_t scaleStart;
		volatile int16_t scaleEnd;
		volatile int16_t scaleLeft[5];
		volatile int16_t scaleRight[5];
		volatile int16_t scaleXLeft[5];
		volatile int16_t scaleXRight[5];
		volatile int16_t scaleY[5];

		volatile int16_t scaleMainWidth;
		volatile int16_t scaleLargeWidth;
		volatile int16_t scaleSmallWidth;
		volatile int16_t scaleLargeLength;
		volatile int16_t scaleSmallLength;
		volatile int16_t scaleLargeSteps;
		volatile int16_t scaleSmallSteps;
		volatile int16_t scaleAccColorEvery;
		volatile int16_t scaleTextSteps;
		volatile int16_t scaleAntialiasing;
		volatile int16_t internalEllipseDistance;

		volatile uint32_t backgroundColor;
		volatile uint32_t scaleColor;
		volatile uint32_t scaleAccColor;
		volatile uint32_t needleCenterColor;
		volatile uint32_t fontColor;
		volatile uint32_t iconColor;
		volatile uint32_t needleColor;
		
		volatile InputData input[6];

};

#endif
