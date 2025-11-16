#ifndef CONFIG_H
#define CONFIG_H

// =================== PIN DEFINITIONS ===================
#define RAIN_A0 34   // Analog from rain module (ADC1)
#define RAIN_D0 26   // Digital output (potentiometer threshold)
#define I2C_SDA 21   // I2C Data
#define I2C_SCL 22   // I2C Clock
#define SERVO_PIN 13 // SG90 servo signal
#define HALL_PIN 32  // Hall sensor (wind anemometer)

// =================== WIND SENSOR SETTINGS ===================
#define PPR 1                 // Pulses per revolution (1 magnet = 1 pulse)
#define MIN_PULSE_US 20000    // Debounce: ignore pulses <20ms apart
#define WIND_TIMEOUT_MS 10000 // No pulse for this long → wind=0
#define RADIUS_M 0.06f        // Cup radius (meters) from axis to magnet
#define K_FACTOR 1.10f        // Calibration factor (tune after testing)

// =================== RAIN SENSOR SETTINGS ===================
#define LOGIC_PERIOD_MS 80 // Rain sensor polling interval
#define AVG_SAMPLES 4      // Number of analog samples to average
#define WET_MARGIN 80      // Initial wet reference margin
#define TRIGGER_DROP 90    // Instant wet trigger threshold
#define DRY_HYST 80        // Hysteresis for dry calibration drift
#define MIN_DENOM 80       // Minimum denominator for percentage calc
#define PCT_EASE_WET 0.65f // EMA alpha when wet
#define PCT_EASE_DRY 0.25f // EMA alpha when dry

// =================== SERVO SETTINGS ===================
#define SERVO_ANGLE_DRY 100   // Servo angle when dry (adjust if not straight)
#define SERVO_ANGLE_WET 5     // Servo angle when wet (adjust if crooked)
#define SERVO_MIN_ON_MS 10000 // Stay at wet angle at least 10s once wet
#define SERVO_MIN_OFF_MS 5000 // Stay at dry angle at least 5s once dry
#define WET_DEBOUNCE_MS 800   // ~0.8s continuous wet before switching on
#define DRY_DEBOUNCE_MS 1200  // ~1.2s continuous dry before switching off

// =================== WIFI & MQTT SETTINGS ===================
// --Netwerk hotspot--
#define WIFI_SSID "REDACTED"
#define WIFI_PASS "REDACTED"
#define MQTT_BROKER_ADDRESS "145.24.237.211"
#define MQTT_BROKER_PORT 8883
#define MQTT_CLIENT_ID "weather_station_01"
#define MQTT_USER "minor_smart_things"
#define MQTT_PASS "smart_things_2025"

// MQTT Topics
#define MQTT_TOPIC_GPS "homestations/1053258/1/gps"
#define MQTT_TOPIC_WIND "homestations/1053258/1/windspeed"
#define MQTT_TOPIC_RAIN "homestations/1053258/1/rain"
#define MQTT_TOPIC_TEMP "homestations/1053258/1/temperature"
#define MQTT_TOPIC_HUM "homestations/1053258/1/humidity"
#define MQTT_TOPIC_PRESSURE "homestations/1053258/1/airpressure"
#define MQTT_TOPIC_MOTOR "homestations/1053258/1/motor"
#define MQTT_TOPIC_UPDATE "homestations/1053258/1/update"

#define PUBLISH_INTERVAL 10000 // Publish every 10 seconds

// =================== UI THEME COLORS ===================
#define COL_BG 0x0000      // TFT_BLACK
#define COL_SURFACE 0x0841 // dark grey-blue
#define COL_BORDER 0x4208  // TFT_DARKGREY
#define COL_ACCENT 0x027F  // cyan-ish
#define COL_GOOD 0x07E0    // TFT_GREEN
#define COL_BAD 0xF800     // TFT_RED
#define COL_TEXT 0xFFFF    // TFT_WHITE
#define COL_MUTE 0xB596    // TFT_LIGHTGREY
#define COL_NUMBER 0x06BF  // bright cyan

#endif // CONFIG_H
