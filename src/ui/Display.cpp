#include "Display.h"
#include "Feedback.h"        // for extern MKRIoTCarrier carrier

#include <Arduino.h>
#include <Arduino_MKRIoTCarrier.h>

// Display dimensions (240 × 240 px)
static constexpr int16_t DISP_W = 240;
static constexpr int16_t DISP_H = 240;

// ── Internal state ────────────────────────────────────────────────────────────
static SystemState  _dState    = SystemState::DISARMED;
static String       _dSubtitle = "";
static bool         _dirty     = false;

// ── Color helpers ─────────────────────────────────────────────────────────────
static uint16_t _bgColor(SystemState s) {
    switch (s) {
        case SystemState::DISARMED:  return carrier.display.color565(0,  50,  0);
        case SystemState::ARMED:     return carrier.display.color565(0,   0, 70);
        case SystemState::PRE_ALARM: return carrier.display.color565(70, 35,  0);
        case SystemState::TRIGGERED: return carrier.display.color565(80,  0,  0);
        default:                     return 0x0000;  // black
    }
}

// Center a string horizontally given a text size (Adafruit GFX char cell = 6×8,
// scaled by textSize; each character is textSize*6 pixels wide).
static int16_t _centerX(const char* text, uint8_t textSize) {
    int16_t px = static_cast<int16_t>(strlen(text)) * textSize * 6;
    return (DISP_W - px) / 2;
}

// ── Render ────────────────────────────────────────────────────────────────────
static void _render() {
    carrier.display.fillScreen(_bgColor(_dState));
    carrier.display.setTextWrap(false);
    carrier.display.setTextColor(0xFFFF);  // white

    // ── Kit ID (size 2, top, y = 18) ─────────────────────────────────────
    carrier.display.setTextSize(2);
    carrier.display.setCursor(_centerX(KIT_ID, 2), 18);
    carrier.display.print(KIT_ID);

    // ── State name (size 3, middle, y = 90) ──────────────────────────────
    const char* name = stateName(_dState);
    carrier.display.setTextSize(3);
    carrier.display.setCursor(_centerX(name, 3), 90);
    carrier.display.print(name);

    // ── Subtitle (size 2, lower third, y = 160) ──────────────────────────
    if (_dSubtitle.length() > 0) {
        carrier.display.setTextSize(2);
        carrier.display.setCursor(_centerX(_dSubtitle.c_str(), 2), 160);
        carrier.display.print(_dSubtitle);
    }
}

// ── Public API ────────────────────────────────────────────────────────────────
void displayBegin() {
    carrier.display.fillScreen(0x0000);
}

void displaySetState(SystemState s, const char* subtitle) {
    _dState    = s;
    _dSubtitle = subtitle ? subtitle : "";
    _dirty     = true;
}

void displayTick() {
    if (!_dirty) return;
    _dirty = false;
    _render();
}
