#ifndef RAIN_SENSOR_H
#define RAIN_SENSOR_H

#include <Arduino.h>
#include <ESP32Servo.h>

// Initialize rain sensor and servo
void rainSensorInit(Servo &servo);

// Update rain sensor readings and servo position (call periodically)
void rainSensorUpdate(Servo &servo);

// Get current rain state
bool isRaining();

// Get wetness percentage
float getWetnessPercent();

// Get raw analog reading
uint16_t getRainRaw();

#endif // RAIN_SENSOR_H
