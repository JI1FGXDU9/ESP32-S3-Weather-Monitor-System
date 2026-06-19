#pragma once

#include "Config.h"

bool initSensors();
bool readSensors();

extern float shadeTemp;
extern float sunTemp;
extern float humidity;
extern float pressure;

extern int sensorMode;