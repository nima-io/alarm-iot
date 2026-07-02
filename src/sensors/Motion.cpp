#include "Motion.h"
#include "../config/Timings.h"

#include <Arduino.h>

// Digital pin wired to the HC-SR501 PIR output.
// The Opla IoT Kit routes the PIR to D6; change here if rewired.
static constexpr uint8_t PIR_PIN = 6;

static bool     _lastLevel = false;   // previous digitalRead level
static uint32_t _lastEvent = 0;       // millis() of the last reported event

void motionBegin() {
    pinMode(PIR_PIN, INPUT);
}

bool motionTick() {
    const bool level  = (digitalRead(PIR_PIN) == HIGH);
    const bool rising = level && !_lastLevel;
    _lastLevel = level;

    if (!rising) return false;

    // Rising edge detected — honour the debounce window
    const uint32_t now = millis();
    if (now - _lastEvent >= PIR_DEBOUNCE_MS) {
        _lastEvent = now;
        return true;
    }
    return false;
}
