#ifndef UI_H
#define UI_H

#include <TFT_eSPI.h>

// Initialize the UI (display, layout, draw static elements)
void uiInit(TFT_eSPI &tft);

// Update dynamic UI elements (call periodically)
void uiUpdate(TFT_eSPI &tft, bool isDry, float wetnessPct, float windMs, float tempC, float humidityPct, float pressureHPa);

// Heartbeat indicator
void uiHeartbeat(TFT_eSPI &tft);

#endif // UI_H
