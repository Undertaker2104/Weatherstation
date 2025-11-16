#ifndef WIND_SENSOR_H
#define WIND_SENSOR_H

#include <Arduino.h>

struct WindSample
{
    float rpm;
    float ms; // meters per second
};

// Initialize wind sensor (attach interrupt, etc.)
void windSensorInit();

// Read current wind speed (call periodically)
WindSample readWind();

// Get pulse count (for debugging)
int getWindPulseCount();

#endif // WIND_SENSOR_H
