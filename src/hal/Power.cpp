#include "Power.h"
#include "hal/Display.h"
#include "hal/Keyboard.h"


// Forward declarations — display & keyboard instances provided externally
extern Display display;
extern Keyboard keyboard;

// Battery type lookup table for getting voltage vs percent
// interpolated chart from https://www.researchgate.net/figure/Li-ion-battery-discharge-voltage-curve_fig5_363575973
// the manual chart reading entries are read from chart, while the others are interpolated
namespace {

    struct VoltPoint { float v; int pct; };
    static constexpr VoltPoint LIPO_CURVE[] = {
        {3.90f, 100},
        {3.80f, 90},
        {3.72f, 80},
        {3.65f, 70},
        {3.59f, 60},
        {3.53f, 50},
        {3.48f, 40},
        {3.44f, 30},
        {3.40f, 20},
        {3.36f, 15},
        {3.30f, 10},
        {3.15f, 5},
        {3.00f, 0},
    };
    constexpr int LIPO_CURVE_N = sizeof(LIPO_CURVE) / sizeof(LIPO_CURVE[0]);
}






void Power::enablePeripherals() {
    // CRITICAL: GPIO 10 must be HIGH to enable all T-Deck Plus peripherals
    pinMode(BOARD_POWER_PIN, OUTPUT);
    digitalWrite(BOARD_POWER_PIN, HIGH);
    delay(10);  // Allow peripherals to stabilize
}

void Power::begin() {
    _lastActivity = millis();
    _state = ACTIVE;

    // Configure battery ADC
    pinMode(BAT_ADC_PIN, INPUT);
    analogReadResolution(12);

    Serial.println("[POWER] Power manager initialized");
}

float Power::batteryVoltage() const {
    // T-Deck Plus: voltage divider on GPIO 4
    int raw = analogRead(BAT_ADC_PIN);
    // Voltage divider: 2x ratio, 3.3V reference, 12-bit ADC
    return (raw / 4095.0f) * 3.3f * 2.0f;
}

int Power::batteryPercent() const {
    float v = batteryVoltage();

    // Charging: voltage exceeds the discharge curve's top (3.9V = full).
    // A discharging cell never gets there, so treat this as full.
    if (isCharging())
        return 100;

    // Compensate for load-induced voltage drop when running on battery.
    // Calculates an offset and adds it to real voltage for to be able using default LiPo lookup table
    v += (3.9f - _fullBatteryV);

    // Clamp to valid curve range.
    v = constrain(v, 3.0f, 4.2f);

    if (_batteryModel == 1) {
        // Linear: distribution across 3.0–4.2V.
        return (int)((v - 3.0f) / 1.2f * 100.0f);
    }

    // LiPo: interpolate between nearest table entries.
    for (int i = 0; i < LIPO_CURVE_N - 1; i++) {
        if (v >= LIPO_CURVE[i + 1].v) {
            float t = (v - LIPO_CURVE[i + 1].v) / (LIPO_CURVE[i].v - LIPO_CURVE[i + 1].v);
            return (int)(LIPO_CURVE[i + 1].pct + t * (LIPO_CURVE[i].pct - LIPO_CURVE[i + 1].pct));
        }
    }
    return 0;
}

void Power::setBatteryModel(uint8_t model) {
    _batteryModel = model;
}

bool Power::isCharging() const {
    return batteryVoltage() >= _chargeThreshold;
}

void Power::setChargeThreshold(float v)   {
    _chargeThreshold = v;
}

void Power::setFullBatteryVoltage(float v) {
    _fullBatteryV = v;
}


uint8_t Power::percentToPWM(uint8_t pct) const {
    if (pct == 0) return 0;
    if (pct >= 100) return 255;
    // Map 1-100 to ~6-255 (minimum visible PWM ~6)
    return (uint8_t)(6 + (uint16_t)(pct - 1) * 249 / 99);
}

void Power::activity() {
    _lastActivity = millis();
    if (_state == SCREEN_OFF) {
        _justWokeFromOff = true;
    }
    if (_state != ACTIVE) {
        setState(ACTIVE);
    }
}

void Power::forceScreenOff() {
    if (_justWokeFromOff) {
        _justWokeFromOff = false;
        return;
    }
    setState(SCREEN_OFF);
}

void Power::weakActivity() {
    _lastActivity = millis();
    // Trackball wakes from DIM but not from SCREEN_OFF
    if (_state == DIMMED) {
        setState(ACTIVE);
    }
}

void Power::setBrightness(uint8_t percent) {
    _brightnessPct = constrain(percent, 1, 100);
    if (_state == ACTIVE) {
        display.setBrightness(percentToPWM(_brightnessPct));
    }
}

void Power::setKbBrightness(uint8_t percent, bool apply) {
    percent = constrain(percent, 0, 100);
    keyboard.setBacklightBrightness(percent);
    if (percent == 0) {
        keyboard.backlightOff();
    } else if (apply) { // Show the new brightness
        keyboard.backlightOn();
    }
}

void Power::loop() {
    unsigned long elapsed = millis() - _lastActivity;

    switch (_state) {
        case ACTIVE:
            if (_offTimeout > 0 && elapsed >= _offTimeout) {
                setState(SCREEN_OFF);
            } else if (_dimTimeout > 0 && elapsed >= _dimTimeout) {
                setState(DIMMED);
            }
            break;

        case DIMMED:
            if (_offTimeout > 0 && elapsed >= _offTimeout) {
                setState(SCREEN_OFF);
            }
            break;

        case SCREEN_OFF:
            break;
    }

    _justWokeFromOff = false;
}

void Power::setState(State newState) {
    if (newState == _state) return;
    const char* names[] = {"ACTIVE", "DIMMED", "SCREEN_OFF"};
    Serial.printf("[POWER] %s -> %s\n", names[_state], names[newState]);
    State oldState = _state;
    _state = newState;

    switch (_state) {
        case ACTIVE:
            if (oldState == SCREEN_OFF) {
                // Pre-load correct brightness into LovyanGFX state before wakeup.
                // wakeup() sends SLPOUT then restores LGFX's internal _brightness
                // to the LEDC — with this ordering, it restores the correct value
                // instead of a stale 0, eliminating the rapid 0→0→correct triple-
                // write that can cause missed LEDC duty updates on ESP32-S3.
                display.setBrightness(percentToPWM(_brightnessPct));
                display.wakeup();
            } else {
                display.setBrightness(percentToPWM(_brightnessPct));
            }
            // On wake, relight only what screen-off forced dark (or per auto-on) —
            // never force-enable for users who keep the kb light off.
            if (_kbAutoOn || (oldState == SCREEN_OFF && _kbLitBeforeOff)) {
                keyboard.backlightOn();
            }
            break;
        case DIMMED:
            display.setBrightness(DIM_PWM);
            if (_kbAutoOff) {
                keyboard.backlightOff();
            }
            break;
        case SCREEN_OFF:
            // LovyanGFX sleep() sets brightness to 0 internally — no
            // need to call setBrightness(0) beforehand.
            display.sleep();
            // Kb backlight always follows screen-off — the timeout exists to save battery.
            _kbLitBeforeOff = keyboard.backlightIsLit();
            keyboard.backlightOff();
            break;
    }
}
