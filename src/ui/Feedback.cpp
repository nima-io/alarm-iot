#include "Feedback.h"
#include "../config/Timings.h"

#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

// Single definition of the global carrier object (Display.cpp uses extern).
MKRIoTCarrier carrier;

// ── Internal state ────────────────────────────────────────────────────────────
static SystemState _state    = SystemState::DISARMED;

// LED pulse animation
static uint32_t    _ledTimer  = 0;
static bool        _ledBright = true;

// Buzzer state machine
enum class BuzzerMode : uint8_t { OFF, ENTRY, CHIRP, SIREN };
static BuzzerMode  _bMode    = BuzzerMode::OFF;
static bool        _bOn      = false;
static uint32_t    _bTimer   = 0;
static uint32_t    _chirpStart = 0;
static bool        _sirenHigh  = true;

// ── Helpers ───────────────────────────────────────────────────────────────────
static void _setAllLeds(uint8_t r, uint8_t g, uint8_t b) {
    for (int i = 0; i < 5; i++) {
        carrier.leds.setPixelColor(i, r, g, b);
    }
    carrier.leds.show();
}

static void _stopBuzzer() {
    carrier.Buzzer.noSound();
    _bOn      = false;
    _bMode    = BuzzerMode::OFF;
    _chirpStart = 0;
}

// ── Public API ────────────────────────────────────────────────────────────────
void feedbackBegin() {
    CARRIER_CASE = false;
    carrier.begin();

    carrier.leds.setBrightness(120);
    _setAllLeds(0, 20, 0);   // green dim → DISARMED
}

void feedbackApplyState(SystemState s) {
    _state     = s;
    _ledBright = true;
    _ledTimer  = millis();
    _stopBuzzer();

    switch (s) {
        case SystemState::DISARMED:
            _setAllLeds(0, 20, 0);          // green dim
            break;

        case SystemState::ARMED:
            _setAllLeds(0, 0, 180);         // blue
            break;

        case SystemState::PRE_ALARM:
            // Kit A: amber LEDs + short entry beep.
            // Kit B: Defuse module will immediately override LEDs to red via
            //        feedbackSetLed() when defuseArm() is called (Phase 5).
            _setAllLeds(255, 80, 0);        // amber
            carrier.Buzzer.sound(440);
            _bMode  = BuzzerMode::ENTRY;
            _bTimer = millis();
            break;

        case SystemState::TRIGGERED:
            _setAllLeds(255, 0, 0);         // red
            carrier.Buzzer.sound(880);
            _bOn      = true;
            _bMode    = BuzzerMode::SIREN;
            _bTimer   = millis();
            _sirenHigh = true;
            break;
    }
}

void feedbackTick() {
    const uint32_t now = millis();

    // ── LED pulse animation ────────────────────────────────────────────────
    // Kit B hands LED control to the Defuse module in PRE_ALARM / TRIGGERED,
    // so pulsing is skipped there. Kit A pulses in both alarm states.
#ifndef ROLE_KEYPAD
    if (_state == SystemState::PRE_ALARM || _state == SystemState::TRIGGERED) {
        if (now - _ledTimer >= 400) {
            _ledTimer  = now;
            _ledBright = !_ledBright;
            if (_state == SystemState::PRE_ALARM) {
                _setAllLeds(_ledBright ? 255 : 100, _ledBright ? 80 : 30, 0);
            } else {
                _setAllLeds(_ledBright ? 255 : 80, 0, 0);
            }
        }
    }
#endif

    // ── Buzzer state machine ───────────────────────────────────────────────
    switch (_bMode) {
        case BuzzerMode::OFF:
            break;

        case BuzzerMode::ENTRY:
            // Single 200 ms beep played on entering PRE_ALARM
            if (now - _bTimer >= 200) {
                carrier.Buzzer.noSound();
                _bMode = BuzzerMode::OFF;
                _bOn   = false;
            }
            break;

        case BuzzerMode::CHIRP: {
            // One 100 ms beep per second for EXIT_DELAY_MS
            const uint32_t elapsed = now - _chirpStart;
            if (elapsed >= EXIT_DELAY_MS) {
                carrier.Buzzer.noSound();
                _bMode      = BuzzerMode::OFF;
                _bOn        = false;
                _chirpStart = 0;
                break;
            }
            const uint32_t cycle = elapsed % 1000;
            if (cycle < 100 && !_bOn) {
                carrier.Buzzer.sound(1000);
                _bOn = true;
            } else if (cycle >= 100 && _bOn) {
                carrier.Buzzer.noSound();
                _bOn = false;
            }
            break;
        }

        case BuzzerMode::SIREN:
            // Alternate between two tones every 500 ms
            if (now - _bTimer >= 500) {
                _bTimer    = now;
                _sirenHigh = !_sirenHigh;
                carrier.Buzzer.sound(_sirenHigh ? 880 : 660);
            }
            break;
    }
}

void feedbackChirpArming() {
    _stopBuzzer();
    _chirpStart = millis();
    _bMode      = BuzzerMode::CHIRP;
    // Play the first beep immediately; feedbackTick() will handle the rest.
    carrier.Buzzer.sound(1000);
    _bOn = true;
}

void feedbackSetLed(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= 5) return;
    carrier.leds.setPixelColor(index, r, g, b);
    carrier.leds.show();
}
