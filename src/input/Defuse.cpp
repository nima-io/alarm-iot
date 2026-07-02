#include "Defuse.h"
#include "../ui/Feedback.h"   // feedbackSetLed() + extern MKRIoTCarrier carrier
#include "../net/Mqtt.h"
#include "../config/Topics.h"

#include <Arduino.h>
#include <ArduinoJson.h>

// ── Internal state ────────────────────────────────────────────────────────────
enum class DefuseState { IDLE, ACTIVE, WRONG_FLASH };

static DefuseState _ds         = DefuseState::IDLE;
static char        _pin[6]     = {'\0'};  // null-terminated copy of the PIN
static uint8_t     _progress   = 0;       // correct pads entered so far (0-5)

// Correct-press beep timer (buzzer on for CORRECT_BEEP_MS then off)
static uint32_t    _beepTimer  = 0;
static bool        _beepOn     = false;

// Wrong-press flash timer (white LEDs + high beep for WRONG_FLASH_MS)
static uint32_t    _flashTimer = 0;

static constexpr uint32_t CORRECT_BEEP_MS = 150;
static constexpr uint32_t WRONG_FLASH_MS  = 300;

// ── Helpers ───────────────────────────────────────────────────────────────────
// Light all 5 pads red — represents "all targets still active".
static void _armLeds() {
    for (uint8_t i = 0; i < 5; i++) {
        feedbackSetLed(i, 255, 0, 0);
    }
}

// ── Public API ────────────────────────────────────────────────────────────────
void defuseBegin(const char* pin) {
    strncpy(_pin, pin, 5);
    _pin[5] = '\0';
    _ds     = DefuseState::IDLE;
}

void defuseArm() {
    // Flush any stale touch edges accumulated while defuse was idle.
    carrier.Buttons.update();
    _progress = 0;
    _beepOn   = false;
    _ds       = DefuseState::ACTIVE;
    _armLeds();
}

void defuseDisarm() {
    _ds = DefuseState::IDLE;
    if (_beepOn) {
        carrier.Buzzer.noSound();
        _beepOn = false;
    }
}

void defuseTick() {
    if (_ds == DefuseState::IDLE) return;

    const uint32_t now = millis();

    // ── End correct-press beep ────────────────────────────────────────────
    if (_beepOn && now - _beepTimer >= CORRECT_BEEP_MS) {
        carrier.Buzzer.noSound();
        _beepOn = false;
    }

    // ── Wrong-flash timeout ───────────────────────────────────────────────
    if (_ds == DefuseState::WRONG_FLASH) {
        if (now - _flashTimer >= WRONG_FLASH_MS) {
            carrier.Buzzer.noSound();   // end the high beep
            _ds = DefuseState::ACTIVE;
            _armLeds();                 // restore all-red (progress was reset to 0)
        }
        return;   // don't process presses during the flash
    }

    // ── Poll touch pads (ACTIVE) ──────────────────────────────────────────
    carrier.Buttons.update();

    // touchButtons is an enum — map uint8_t index to the expected type.
    static const touchButtons kPads[5] = { TOUCH0, TOUCH1, TOUCH2, TOUCH3, TOUCH4 };

    for (uint8_t pad = 0; pad < 5; pad++) {
        if (!carrier.Buttons.onTouchDown(kPads[pad])) continue;

        // Map PIN character ('1'-'5') to pad index (0-4)
        const uint8_t expected = (uint8_t)(_pin[_progress] - '1');

        if (pad == expected) {
            // ── Correct press ─────────────────────────────────────────────
            feedbackSetLed(pad, 0, 0, 0);   // extinguish this LED
            carrier.Buzzer.sound(440);       // low beep
            _beepOn    = true;
            _beepTimer = now;
            _progress++;

            if (_progress == 5) {
                // All 5 pads entered correctly — defuse complete
                _ds = DefuseState::IDLE;
                JsonDocument doc;
                doc["ts"] = millis();
                publishJson(TOPIC_KIT_B_DEFUSED, doc);
                Serial.println(F("[Defuse] DEFUSED"));
            }
        } else {
            // ── Wrong press ───────────────────────────────────────────────
            for (uint8_t i = 0; i < 5; i++) {
                feedbackSetLed(i, 255, 255, 255);   // flash all white
            }
            carrier.Buzzer.sound(1000);              // high error beep
            _flashTimer = now;
            _ds         = DefuseState::WRONG_FLASH;
            _progress   = 0;

            JsonDocument doc;
            doc["ts"] = millis();
            publishJson(TOPIC_KIT_B_DEFUSE_FAIL, doc);
            Serial.println(F("[Defuse] fail"));
        }
        break;   // process at most one press per tick
    }
}
