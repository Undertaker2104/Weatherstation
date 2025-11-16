# Weather Station - Code Structure

## Overview
The code has been refactored into a modular architecture for better maintainability and clarity.

## File Structure

```
include/
  └── config.h              # All pin definitions, WiFi/MQTT credentials, and tunables

src/
  ├── main.cpp              # Main coordinator (setup & loop)
  ├── wind_sensor.h/cpp     # Wind anemometer (Hall sensor, ISR, readings)
  ├── rain_sensor.h/cpp     # Rain sensor (analog/digital, calibration, servo control)
  ├── ui.h/cpp              # TFT display (drawing functions, layout, theme)
  └── mqtt_client.h/cpp     # WiFi & MQTT (connection, publishing)
```

## Module Responsibilities

### config.h
- Pin assignments for all peripherals
- Wind sensor configuration (PPR, debounce, radius, calibration)
- Rain sensor tuning (thresholds, EMA alpha, hysteresis)
- Servo timing (debounce, dwell times)
- WiFi & MQTT credentials and topics
- UI theme colors

### wind_sensor (wind_sensor.h/cpp)
- Hall sensor interrupt handler
- Pulse counting and timing
- Wind speed calculation (RPM and m/s)
- Calibration formula application

### rain_sensor (rain_sensor.h/cpp)
- Analog ADC reading and averaging
- Digital input polarity detection
- Dry/wet calibration (auto-calibrates at boot)
- Wetness percentage calculation with EMA smoothing
- Servo control with debouncing and dwell timing

### ui (ui.h/cpp)
- TFT display initialization and layout
- Adaptive layout (compact mode for 240px displays)
- Drawing functions for all UI elements:
  - App bar with title
  - DRY/RAIN status chip
  - Wetness percentage pill
  - Wind speed card
  - Temperature/humidity card
  - Pressure card
  - Heartbeat indicator

### mqtt_client (mqtt_client.h/cpp)
- WiFi connection management
- MQTT broker connection and reconnection
- Publishing sensor data to topics
- JSON payload formatting

### main.cpp
- Hardware instance creation (TFT, BME280, Servo)
- Initialization of all modules
- Main loop coordination:
  - Sensor polling (80ms interval)
  - UI updates
  - MQTT publishing (10s interval)
  - Connection maintenance

## How to Modify

### Change WiFi/MQTT settings
Edit `include/config.h` → WiFi & MQTT section

### Tune rain sensor sensitivity
Edit `include/config.h` → Rain sensor settings (TRIGGER_DROP, WET_MARGIN, etc.)

### Adjust servo timing
Edit `include/config.h` → Servo settings (debounce and dwell times)

### Calibrate wind sensor
Edit `include/config.h` → Wind sensor settings (RADIUS_M, K_FACTOR)

### Modify UI layout or colors
Edit `src/ui.cpp` and `include/config.h` (theme colors)

### Add new sensors
1. Create new sensor module (e.g., `gps_sensor.h/cpp`)
2. Add init call in `main.cpp setup()`
3. Add update call in `main.cpp loop()`
4. Update UI and MQTT modules as needed

## Building and Uploading

```powershell
# Build
platformio run

# Upload
platformio run -t upload

# Monitor serial output
platformio device monitor -b 115200
```

## Backup
The original monolithic `main.cpp` has been saved as `main_old_backup.cpp`.
