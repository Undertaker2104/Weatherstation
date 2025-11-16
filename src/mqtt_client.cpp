#include "mqtt_client.h"
#include "config.h"
#include <WiFi.h>
#include <ArduinoJson.h>

static WiFiClient network;
static PubSubClient mqtt(network);
static unsigned long lastPublishTime = 0;
static Servo *servoPtr = nullptr; // Pointer to servo for callback access

static void connectWiFi()
{
    Serial.print("Connecting to WiFi");
    WiFi.mode(WIFI_STA); // Set WiFi to station mode
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    int attempts = 0;
    while (WiFi.status() != WL_CONNECTED && attempts < 20)
    {
        delay(500);
        Serial.print(".");
        attempts++;
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWiFi connected!");
        Serial.print("IP: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        Serial.println("\nWiFi connection failed!");
    }
}

// Callback function for incoming MQTT messages
static void mqttMessageCallback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message received on topic: ");
    Serial.println(topic);

    // Parse the message
    String message = "";
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    Serial.print("Payload: ");
    Serial.println(message);

    // Check if it's the motor topic
    if (String(topic) == String(MQTT_TOPIC_MOTOR))
    {
        // Parse JSON or simple value
        int motorCommand = message.toInt();

        if (motorCommand == 1 && servoPtr != nullptr)
        {
            servoPtr->write(SERVO_ANGLE_WET);
            Serial.printf("Motor command: ON (%d°)\n", SERVO_ANGLE_WET);
        }
        else if (motorCommand == 0 && servoPtr != nullptr)
        {
            servoPtr->write(SERVO_ANGLE_DRY);
            Serial.printf("Motor command: OFF (%d°)\n", SERVO_ANGLE_DRY);
        }
    }
}

static void connectMQTT()
{
    int attempts = 0;
    const int maxAttempts = 3; // Only try 3 times during setup

    while (!mqtt.connected() && attempts < maxAttempts)
    {
        Serial.print("Connecting to MQTT...");
        if (mqtt.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS))
        {
            Serial.println("connected!");

            // Subscribe to motor topic to receive commands
            if (mqtt.subscribe(MQTT_TOPIC_MOTOR))
            {
                Serial.print("Subscribed to: ");
                Serial.println(MQTT_TOPIC_MOTOR);
            }
            else
            {
                Serial.println("Failed to subscribe to motor topic!");
            }
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqtt.state());
            Serial.println(" retrying in 2s");
            delay(2000);
            attempts++;
        }
    }

    if (!mqtt.connected())
    {
        Serial.println("MQTT connection failed after 3 attempts. Continuing without MQTT...");
    }
}

void mqttInit(Servo &servo)
{
    servoPtr = &servo; // Store servo pointer for callback access

    connectWiFi();

    mqtt.setServer(MQTT_BROKER_ADDRESS, MQTT_BROKER_PORT);
    mqtt.setCallback(mqttMessageCallback); // Set callback for incoming messages
    connectMQTT();
    Serial.println("MQTT client initialized");
}

void mqttMaintain()
{
    static unsigned long lastReconnectAttempt = 0;
    const unsigned long reconnectInterval = 30000; // Try reconnecting every 30 seconds

    if (WiFi.status() != WL_CONNECTED)
    {
        connectWiFi();
    }

    if (!mqtt.connected())
    {
        unsigned long now = millis();
        if (now - lastReconnectAttempt >= reconnectInterval)
        {
            lastReconnectAttempt = now;
            connectMQTT();
        }
    }

    mqtt.loop();
}

bool shouldPublish()
{
    return (millis() - lastPublishTime >= PUBLISH_INTERVAL);
}

void mqttPublishData(float windMs, bool isRaining, float tempC, float humidity, float pressure, int servoAngle)
{
    if (!mqtt.connected())
    {
        // Update lastPublishTime to prevent spam
        lastPublishTime = millis();
        return; // Silently skip if not connected
    }

    char buffer[64];

    Serial.println("\n=== Publishing Sensor Data ===");

    // Wind speed (raw float)
    snprintf(buffer, sizeof(buffer), "%.2f", windMs);
    mqtt.publish(MQTT_TOPIC_WIND, buffer);
    Serial.printf("  Wind: %s\n", buffer);

    // Rain (raw boolean: "true" or "false")
    mqtt.publish(MQTT_TOPIC_RAIN, isRaining ? "true" : "false");
    Serial.printf("  Rain: %s\n", isRaining ? "true" : "false");

    // Temperature (raw float)
    snprintf(buffer, sizeof(buffer), "%.2f", tempC);
    mqtt.publish(MQTT_TOPIC_TEMP, buffer);
    Serial.printf("  Temp: %s\n", buffer);

    // Humidity (raw float)
    snprintf(buffer, sizeof(buffer), "%.2f", humidity);
    mqtt.publish(MQTT_TOPIC_HUM, buffer);
    Serial.printf("  Humidity: %s\n", buffer);

    // Pressure (raw float)
    snprintf(buffer, sizeof(buffer), "%.2f", pressure);
    mqtt.publish(MQTT_TOPIC_PRESSURE, buffer);
    Serial.printf("  Pressure: %s\n", buffer);

    // Motor/servo angle (raw int)
    snprintf(buffer, sizeof(buffer), "%d", servoAngle);
    mqtt.publish(MQTT_TOPIC_MOTOR, buffer);
    Serial.printf("  Motor: %s\n", buffer);

    // Send update notification (raw string)
    mqtt.publish(MQTT_TOPIC_UPDATE, "updated");
    Serial.println("  Update: updated");

    lastPublishTime = millis();
    Serial.println("=== Publish Complete ===\n");
}

void mqttPublishGPS(float latitude, float longitude)
{
    if (!mqtt.connected())
    {
        Serial.println("MQTT not connected, skipping GPS publish");
        return;
    }

    char buffer[64];

    // GPS coordinates as "lat,lon" format
    snprintf(buffer, sizeof(buffer), "%.8f,%.8f", latitude, longitude);
    mqtt.publish(MQTT_TOPIC_GPS, buffer);

    Serial.print("Published GPS coordinates: ");
    Serial.println(buffer);
}
