#include "rain_sensor.h"
#include "config.h"

// --- Auto-cal references ---
static uint16_t dryRef = 3500; // learned at boot
static uint16_t wetRef = 1500; // tracks "wettest recent"

// Digital polarity: true if digital pin reads LOW when wet (common)
static bool digitalWetIsLow = true;

// --- State / animation ---
static float dispPct = 0.0f;
static uint32_t lastChangeMs = 0;
static bool stateWet = false;

// === Servo stability (debounce + dwell) ===
static uint32_t wetSeenSince = 0;
static uint32_t drySeenSince = 0;
static uint32_t servoLastChange = 0;
static bool servoAtWet = false; // true => 90°, false => 0°

// Helper: average multiple analog reads
static uint16_t avgRead(int pin, int n = AVG_SAMPLES)
{
    uint32_t s = 0;
    for (int i = 0; i < n; i++)
    {
        s += analogRead(pin);
        delay(2);
    }
    return s / n;
}

void rainSensorInit(Servo &servo)
{
    // ADC for rain
    analogSetWidth(12);
    analogSetPinAttenuation(RAIN_A0, ADC_11db);
    pinMode(RAIN_D0, INPUT_PULLUP); // avoid floating

    // Auto-cal DRY at boot (keep plate dry ~0.4–0.5s)
    uint32_t sum = 0;
    for (int i = 0; i < 30; i++)
    {
        sum += analogRead(RAIN_A0);
        delay(8);
    }
    dryRef = sum / 30;
    if (dryRef >= 4000)
    {
        Serial.println("WARN: ADC saturated at boot; using fallback dryRef=3500.");
        dryRef = 3500;
    }
    wetRef = (dryRef > WET_MARGIN) ? dryRef - WET_MARGIN : (dryRef > 20 ? dryRef - 20 : dryRef);

    lastChangeMs = millis();

    // Simple digital polarity auto-detect (optional safety)
    uint32_t s2 = 0;
    for (int i = 0; i < 12; i++)
    {
        s2 += analogRead(RAIN_A0);
        delay(4);
    }
    uint16_t rawavg = s2 / 12;
    int d0sample = digitalRead(RAIN_D0);
    if (rawavg < 4000 && abs((int)rawavg - (int)dryRef) < 150)
    {
        digitalWetIsLow = !(d0sample == 0);
    }
    else
    {
        digitalWetIsLow = true; // assume LOW==wet (typical)
    }

    stateWet = digitalWetIsLow ? (digitalRead(RAIN_D0) == 0) : (digitalRead(RAIN_D0) == 1);

    // Servo init
    servo.setPeriodHertz(50);           // standard analog servo frequency
    servo.attach(SERVO_PIN, 500, 2400); // min/max pulse (us) for SG90
    servo.write(SERVO_ANGLE_DRY);       // Start at dry position
    servoAtWet = false;
    servoLastChange = millis();

    Serial.printf("Rain sensor: dryRef=%u, wetRef=%u, digitalWetIsLow=%d\n",
                  dryRef, wetRef, digitalWetIsLow ? 1 : 0);
}

void rainSensorUpdate(Servo &servo)
{
    // ---- Rain logic ----
    uint16_t raw = avgRead(RAIN_A0);
    int d0 = digitalRead(RAIN_D0);
    bool hwWet = digitalWetIsLow ? (d0 == 0) : (d0 == 1);

    bool dropWet = (raw + TRIGGER_DROP < dryRef);
    bool wetNow = hwWet || dropWet;

    if (wetNow)
    {
        if (raw < wetRef)
            wetRef = raw;
    }
    else
    {
        wetRef = (uint16_t)(wetRef + (dryRef - wetRef) / 300);
    }

    if (!wetNow && raw > (uint16_t)(dryRef - DRY_HYST) && raw > dryRef)
    {
        dryRef = (uint16_t)((199 * dryRef + raw) / 200);
    }

    int denom = (int)dryRef - (int)wetRef;
    if (denom < MIN_DENOM)
        denom = MIN_DENOM;
    float targetPct = 100.0f * (float)(dryRef - raw) / (float)denom;
    targetPct = constrain(targetPct, 0.0f, 100.0f);

    float ease = wetNow ? PCT_EASE_WET : PCT_EASE_DRY;
    dispPct += (targetPct - dispPct) * ease;

    if (wetNow != stateWet)
    {
        stateWet = wetNow;
        lastChangeMs = millis();
    }

    // === Stable servo control ===
    uint32_t now = millis();
    if (wetNow)
    {
        if (wetSeenSince == 0)
            wetSeenSince = now;
        drySeenSince = 0;
    }
    else
    {
        if (drySeenSince == 0)
            drySeenSince = now;
        wetSeenSince = 0;
    }

    bool requestWet = (wetSeenSince != 0) && (now - wetSeenSince >= WET_DEBOUNCE_MS);
    bool requestDry = (drySeenSince != 0) && (now - drySeenSince >= DRY_DEBOUNCE_MS);
    bool canLeaveWet = (now - servoLastChange >= SERVO_MIN_ON_MS);
    bool canLeaveDry = (now - servoLastChange >= SERVO_MIN_OFF_MS);

    if (!servoAtWet && requestWet && canLeaveDry)
    {
        servo.write(SERVO_ANGLE_WET);
        servoAtWet = true;
        servoLastChange = now;
        Serial.printf("Servo -> %d (WET)\n", SERVO_ANGLE_WET);
    }
    if (servoAtWet && requestDry && canLeaveWet)
    {
        servo.write(SERVO_ANGLE_DRY);
        servoAtWet = false;
        servoLastChange = now;
        Serial.printf("Servo -> %d (DRY)\n", SERVO_ANGLE_DRY);
    }
}

bool isRaining()
{
    return stateWet;
}

float getWetnessPercent()
{
    return dispPct;
}

uint16_t getRainRaw()
{
    return avgRead(RAIN_A0);
}
