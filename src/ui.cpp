#include "ui.h"
#include "config.h"

// --- Layout (adaptive) ---
static bool compactMode = false;
static int PAD, APPBAR_H, CHIP_H, PILL_H, CARD_H;
static int GAP_S, GAP_M, GAP_ROW;
static int PILL_X, PILL_Y, PILL_W;
static int CARD_W, CARD_ROW1_Y, CARD_ROW2_Y;
static int lastPillFill = -1;

// --------- Layout compute (adaptive to height) ---------
static void configureLayout(TFT_eSPI &tft)
{
    compactMode = (tft.height() < 300);

    if (compactMode)
    {
        PAD = 6;
        APPBAR_H = 30;
        CHIP_H = 28;
        PILL_H = 18;
        CARD_H = 44;
        GAP_S = 4;
        GAP_M = 8;
        GAP_ROW = 6;
    }
    else
    {
        PAD = 12;
        APPBAR_H = 44;
        CHIP_H = 36;
        PILL_H = 26;
        CARD_H = 58;
        GAP_S = 6;
        GAP_M = 12;
        GAP_ROW = 14;
    }

    PILL_X = PAD;
    PILL_W = tft.width() - 2 * PAD;
    PILL_Y = PAD + APPBAR_H + GAP_S + CHIP_H + GAP_M;

    CARD_ROW1_Y = PILL_Y + PILL_H + (compactMode ? 16 : 30);
    CARD_ROW2_Y = CARD_ROW1_Y + CARD_H + GAP_ROW;
}

static void drawShadowRect(TFT_eSPI &tft, int x, int y, int w, int h, uint16_t fill, uint16_t shadow = COL_SURFACE)
{
    tft.fillRoundRect(x + 1, y + 2, w, h, 10, shadow);
    tft.fillRoundRect(x, y, w, h, 10, fill);
}

static void drawChip(TFT_eSPI &tft, bool isDry)
{
    const char *label = isDry ? "DRY" : "RAIN";
    uint16_t col = isDry ? COL_GOOD : COL_BAD;

    int cx = tft.width() / 2, w = compactMode ? 120 : 136, h = CHIP_H;
    int x = cx - w / 2, y = PAD + APPBAR_H + GAP_S;

    tft.fillRoundRect(x, y, w, h, 16, COL_SURFACE);
    tft.drawRoundRect(x, y, w, h, 16, col);

    if (isDry)
    {
        tft.fillCircle(x + 18, y + h / 2, compactMode ? 7 : 8, COL_GOOD);
    }
    else
    {
        int cx2 = x + 18, cy2 = y + h / 2;
        tft.fillCircle(cx2, cy2 - 4, compactMode ? 5 : 6, COL_BAD);
        tft.fillTriangle(cx2 - 6, cy2 - 2, cx2 + 6, cy2 - 2, cx2, cy2 + 10, COL_BAD);
    }

    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(col, COL_SURFACE);
    tft.setTextFont(1);
    tft.setTextSize(compactMode ? 2 : 3);
    tft.drawString(label, cx + 4, y + h / 2);
    tft.setTextSize(1);
    tft.setTextDatum(TL_DATUM);
}

static void drawAppBar(TFT_eSPI &tft)
{
    drawShadowRect(tft, PAD, PAD, tft.width() - 2 * PAD, APPBAR_H, COL_SURFACE);
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(compactMode ? 1 : 2);
    tft.setTextColor(COL_MUTE, COL_SURFACE);

    // Clear text area first
    int textY = PAD + (compactMode ? 4 : 6);
    tft.fillRect(PAD + 6, textY - 2, 200, compactMode ? 14 : 18, COL_SURFACE);

    tft.drawString("Rain · Wind · BME280", PAD + 8, textY);
}

static void drawPillFrame(TFT_eSPI &tft)
{
    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(compactMode ? 1 : 2);
    tft.setTextColor(COL_TEXT, COL_BG);
    tft.drawString("Wetness", PILL_X, PILL_Y - (compactMode ? 14 : 18));

    tft.fillRoundRect(PILL_X, PILL_Y, PILL_W, PILL_H, PILL_H / 2, COL_SURFACE);
    tft.drawRoundRect(PILL_X, PILL_Y, PILL_W, PILL_H, PILL_H / 2, COL_BORDER);

    for (int i = 0; i <= 10; i++)
    {
        int x = PILL_X + (PILL_W * i) / 10;
        tft.drawLine(x, PILL_Y + PILL_H + 1, x, PILL_Y + PILL_H + (compactMode ? 4 : 6), COL_BORDER);
    }
    tft.setTextColor(COL_MUTE, COL_BG);
    tft.drawString("0%", PILL_X, PILL_Y + PILL_H + (compactMode ? 6 : 8));
    tft.setTextDatum(TR_DATUM);
    tft.drawString("100%", PILL_X + PILL_W, PILL_Y + PILL_H + (compactMode ? 6 : 8));
    tft.setTextDatum(TL_DATUM);
}

static void drawPillFill(TFT_eSPI &tft, float pct)
{
    pct = constrain(pct, 0.0f, 100.0f);
    int fillW = (int)(PILL_W * (pct / 100.0f));
    if (fillW == lastPillFill)
        return;
    lastPillFill = fillW;

    tft.fillRoundRect(PILL_X + 1, PILL_Y + 1, PILL_W - 2, PILL_H - 2, (PILL_H - 2) / 2, COL_SURFACE);
    if (fillW > 2)
    {
        tft.fillRoundRect(PILL_X + 1, PILL_Y + 1, fillW - 2, PILL_H - 2, (PILL_H - 2) / 2, COL_ACCENT);
    }

    int px = PILL_X + PILL_W - 4, py = PILL_Y - (compactMode ? 0 : 2);
    tft.fillRect(px - (compactMode ? 66 : 86), py - (compactMode ? 18 : 22),
                 (compactMode ? 66 : 86), (compactMode ? 18 : 22), COL_BG);
    tft.setTextDatum(TR_DATUM);
    tft.setTextFont(compactMode ? 1 : 2);
    tft.setTextColor(COL_NUMBER, COL_BG);
    tft.drawString(compactMode ? String(pct, 0) + " %" : String(pct, 1) + " %", px, py);
    tft.setTextDatum(TL_DATUM);
}

static void drawCardsFrame(TFT_eSPI &tft)
{
    drawShadowRect(tft, PAD, CARD_ROW1_Y, (tft.width() - 2 * PAD), CARD_H, COL_SURFACE);

    CARD_W = (tft.width() - 3 * PAD) / 2;
    drawShadowRect(tft, PAD, CARD_ROW2_Y, CARD_W, CARD_H, COL_SURFACE);
    drawShadowRect(tft, PAD * 2 + CARD_W, CARD_ROW2_Y, CARD_W, CARD_H, COL_SURFACE);

    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(compactMode ? 1 : 2);
    tft.setTextColor(COL_MUTE, COL_SURFACE);

    // Clear label areas first to prevent ghosting
    tft.fillRect(PAD + 8, CARD_ROW1_Y + (compactMode ? 3 : 5), 50, compactMode ? 12 : 16, COL_SURFACE);
    tft.fillRect(PAD + 8, CARD_ROW2_Y + (compactMode ? 3 : 5), 110, compactMode ? 12 : 16, COL_SURFACE);
    tft.fillRect(PAD * 2 + CARD_W + 8, CARD_ROW2_Y + (compactMode ? 3 : 5), 90, compactMode ? 12 : 16, COL_SURFACE);

    tft.drawString("Wind", PAD + 10, CARD_ROW1_Y + (compactMode ? 4 : 6));
    tft.drawString("Temp / Hum", PAD + 10, CARD_ROW2_Y + (compactMode ? 4 : 6));
    tft.drawString("Pressure", PAD * 2 + CARD_W + 10, CARD_ROW2_Y + (compactMode ? 4 : 6));

    if (!compactMode || tft.height() >= 230)
    {
        tft.setTextDatum(MC_DATUM);
        tft.setTextFont(compactMode ? 1 : 2);
        tft.setTextColor(COL_TEXT, COL_BG);
        tft.drawString("Reset with plate dry to re-calibrate",
                       tft.width() / 2, tft.height() - (compactMode ? 8 : 10));
    }
}

static void updateWindSpeedCard(TFT_eSPI &tft, float ms)
{
    int y = CARD_ROW1_Y;
    int valueY = y + (compactMode ? 22 : 26);
    int valueW = (tft.width() - 2 * PAD) - 20;
    tft.fillRect(PAD + 10, valueY, valueW, (compactMode ? 20 : 24), COL_SURFACE);

    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(1);
    tft.setTextSize(2);
    tft.setTextColor(COL_NUMBER, COL_SURFACE);
    char buf[24];
    snprintf(buf, sizeof(buf), "%.1f m/s", ms);
    tft.drawString(buf, PAD + 10, valueY);
    tft.setTextSize(1);
}

static void updateEnvTempHum(TFT_eSPI &tft, float tC, float rh)
{
    int y = CARD_ROW2_Y;
    tft.fillRect(PAD + 10, y + (compactMode ? 22 : 26), CARD_W - 20,
                 (compactMode ? 20 : 24), COL_SURFACE);

    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(1);
    tft.setTextSize(2);
    tft.setTextColor(COL_NUMBER, COL_SURFACE);
    if (!isnan(tC))
    {
        char buf[48];
        if (compactMode)
            snprintf(buf, sizeof(buf), "%.1fC  %.0f%%", tC, rh);
        else
            snprintf(buf, sizeof(buf), "%.1f°C   %.0f%%", tC, rh);
        tft.drawString(buf, PAD + 10, y + (compactMode ? 22 : 26));
    }
    else
    {
        tft.drawString("-- C  -- %", PAD + 10, y + (compactMode ? 22 : 26));
    }
    tft.setTextSize(1);
}

static void updateEnvPressure(TFT_eSPI &tft, float hPa)
{
    int y = CARD_ROW2_Y;
    int x = PAD * 2 + CARD_W;

    tft.fillRect(x + 10, y + (compactMode ? 22 : 26), CARD_W - 20,
                 (compactMode ? 20 : 24), COL_SURFACE);

    tft.setTextDatum(TL_DATUM);
    tft.setTextFont(1);
    tft.setTextSize(2);
    tft.setTextColor(COL_NUMBER, COL_SURFACE);
    if (!isnan(hPa))
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f hPa", hPa);
        tft.drawString(buf, x + 10, y + (compactMode ? 22 : 26));
    }
    else
    {
        tft.drawString("-- hPa", x + 10, y + (compactMode ? 22 : 26));
    }
    tft.setTextSize(1);
}

void uiInit(TFT_eSPI &tft)
{
    tft.init();
    tft.setRotation(1); // landscape
    tft.fillScreen(COL_BG);

    configureLayout(tft);

    drawAppBar(tft);
    drawChip(tft, true); // placeholder
    drawPillFrame(tft);
    drawCardsFrame(tft);

    Serial.println("UI initialized");
}

void uiUpdate(TFT_eSPI &tft, bool isDry, float wetnessPct, float windMs, float tempC, float humidityPct, float pressureHPa)
{
    static bool lastDry = true;
    if (isDry != lastDry)
    {
        drawChip(tft, isDry);
        lastDry = isDry;
    }

    drawPillFill(tft, wetnessPct);
    updateWindSpeedCard(tft, windMs);
    updateEnvTempHum(tft, tempC, humidityPct);
    updateEnvPressure(tft, pressureHPa);
}

void uiHeartbeat(TFT_eSPI &tft)
{
    static bool on = false;
    on = !on;
    tft.fillCircle(tft.width() - 12, PAD + (compactMode ? 12 : 16),
                   (compactMode ? 3 : 4), on ? COL_ACCENT : COL_BORDER);
}
