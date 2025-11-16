#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <PubSubClient.h>
#include <ESP32Servo.h>

// Initialize WiFi and MQTT connection
void mqttInit(Servo &servo);

// Maintain MQTT connection (reconnect if needed)
void mqttMaintain();

// Publish sensor data to MQTT topics
void mqttPublishData(float windMs, bool isRaining, float tempC, float humidity, float pressure, int servoAngle);

// Publish GPS coordinates once at startup
void mqttPublishGPS(float latitude, float longitude);

// Check if it's time to publish
bool shouldPublish();

#endif // MQTT_CLIENT_H
