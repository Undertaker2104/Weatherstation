#include "wind_sensor.h"
#include "config.h"

// ===== Wind ISR/shared state =====
volatile uint32_t wind_last_us = 0;
volatile int pulses = 0;

void IRAM_ATTR onHall()
{
    uint32_t now = micros();
    uint32_t dt = now - wind_last_us;
    if (dt >= MIN_PULSE_US)
    { // debounce
        wind_last_us = now;
        pulses++;
    }
}

void windSensorInit()
{
    pinMode(HALL_PIN, INPUT); // most modules have on-board pull-up
    attachInterrupt(digitalPinToInterrupt(HALL_PIN), onHall, FALLING);
    Serial.println("Wind sensor initialized on pin " + String(HALL_PIN));
}

WindSample readWind()
{
    static WindSample s{0, 0};
    static uint32_t lastMs = millis();

    // timeout => calculate wind from pulse count over timeout window
    if (millis() - lastMs > WIND_TIMEOUT_MS)
    {
        s.rpm = pulses * 60.0f / PPR / (WIND_TIMEOUT_MS / 1000.0f);
        pulses = 0;
        lastMs = millis();
        // Linear calibration formula: ms = 0.0063 * rpm + 1.9973
        if (s.rpm < 10.0f)
        {
            s.ms = 0.0f; // Onder 10 RPM = geen wind
        }
        else
        {
            s.ms = 0.0063 * s.rpm + 1.9973;
        }
    }
    return s;
}

int getWindPulseCount()
{
    return pulses;
}
