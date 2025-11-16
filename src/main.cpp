#include <Arduino.h>
#include <Wire.h>
#include <TFT_eSPI.h>
#include <Adafruit_BME280.h>
#include <ESP32Servo.h>

#include "config.h"
#include "wind_sensor.h"
#include "rain_sensor.h"
#include "ui.h"
#include "mqtt_client.h"

// === Hardware instances ===
TFT_eSPI tft;
Adafruit_BME280 bme;
Servo servo;

// === State ===
bool bmeOk = false;
uint32_t lastLogic = 0;
uint32_t lastUi = 0;

void setup()
{
  Serial.begin(115200);
  delay(140);
  Serial.println("\n=== Weather Station Starting ===");

  // Initialize UI (display)
  uiInit(tft);

  // Initialize rain sensor and servo
  rainSensorInit(servo);

  // Initialize I2C and BME280
  Wire.begin(I2C_SDA, I2C_SCL);
  bmeOk = bme.begin(0x76, &Wire);
  if (!bmeOk)
  {
    bmeOk = bme.begin(0x77, &Wire);
  }
  Serial.printf("BME280: %s\n", bmeOk ? "OK" : "NOT FOUND");

  // Initialize wind sensor
  windSensorInit();

  // Initialize MQTT (WiFi + MQTT broker connection) - pass servo for remote control
  mqttInit(servo);

  // Publish GPS coordinates once at startup (saved to database)
  mqttPublishGPS(51.81208300695626, 4.516824735424278);

  Serial.println("=== Setup Complete ===\n");
}

void loop()
{
  // Main sensor logic tick (every 80ms)
  if (millis() - lastLogic >= LOGIC_PERIOD_MS)
  {
    lastLogic = millis();

    // Update rain sensor and servo
    rainSensorUpdate(servo);

    // Read wind sensor
    WindSample wind = readWind();

    // Read BME280 environmental sensors
    float tempC = NAN, humidity = NAN, pressure = NAN;
    if (bmeOk)
    {
      tempC = bme.readTemperature();          // °C
      humidity = bme.readHumidity();          // %
      pressure = bme.readPressure() / 100.0f; // Pa → hPa
    }

    // Update UI
    uiUpdate(tft, !isRaining(), getWetnessPercent(), wind.ms, tempC, humidity, pressure);

    // Publish to MQTT if interval elapsed
    if (shouldPublish())
    {
      int servoAngle = isRaining() ? 90 : 0;
      mqttPublishData(wind.ms, isRaining(), tempC, humidity, pressure, servoAngle);
    }
  }

  // UI heartbeat indicator (every 400ms)
  if (millis() - lastUi >= 400)
  {
    lastUi = millis();
    uiHeartbeat(tft);
  }

  // Maintain MQTT connection
  mqttMaintain();
}
