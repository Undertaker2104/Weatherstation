# GitHub Copilot project context — ESP32 Weather Station

This project is a local weather station using:
- ESP32 (Lolin32 board)
- Rain sensor (analog A0 → GPIO34, digital D0 → GPIO26)
- BME280 sensor (I²C: SDA=21, SCL=22)
- TFT 240x320 ILI9341 display (SPI: MOSI=23, MISO=19, SCLK=18, CS=5, DC=27, RST=33)
- SG90 Servo (PWM on GPIO13)
- Optional: sends sensor data to a FastAPI backend via HTTP POST (JSON).

## Functional goal not fully implemented yet
1. Continuously read rain sensor and BME280.
2. Display data on the TFT screen with a clean UI.
3. Move the servo to 180° when rain is detected, else return to 0°.
4. Every 10–15 seconds, send a JSON payload to a FastAPI endpoint:
   ```json
   {
     "temperature": 23.4,
     "humidity": 65.2,
     "pressure": 1012.8,
     "rain": true,
     "wetness_pct": 42.7
   }
