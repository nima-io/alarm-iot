#include <Arduino.h>
#include <ArduinoJson.h>

#include "core/State.h"
#include "config/Topics.h"
#include "net/Network.h"
#include "net/Mqtt.h"
#include "ui/Feedback.h"
#include "ui/Display.h"

// ── Global FSM state (follower; Node-RED is the authority) ────────────────────
static SystemState g_state = SystemState::DISARMED;

// ── MQTT message handler ──────────────────────────────────────────────────────
static void onMqttMessage(const char* topic, const char* payload) {
    if (strcmp(topic, TOPIC_SYSTEM_STATE) == 0) {
        // Payload shape: {"state":"ARMED","since":<ms>}
        JsonDocument doc;
        if (deserializeJson(doc, payload) != DeserializationError::Ok) return;

        const char* stateStr = doc["state"] | "";
        SystemState newState = stateFromString(stateStr);
        if (newState == g_state) return;

        const SystemState prev = g_state;
        g_state = newState;

        // ── Shared UI update ─────────────────────────────────────────────
        feedbackApplyState(g_state);

        const char* subtitle = "";
        switch (g_state) {
            case SystemState::PRE_ALARM: subtitle = "Enter code!"; break;
            case SystemState::TRIGGERED: subtitle = "ALARM!";      break;
            default: break;
        }
        displaySetState(g_state, subtitle);

        // ── Transition side effects ──────────────────────────────────────
        // 10 s exit-delay chirp when arming from idle
        if (prev == SystemState::DISARMED && g_state == SystemState::ARMED) {
            feedbackChirpArming();
        }

        // Role-specific effects (Phases 4 & 5) will be added here.

        Serial.print(F("[FSM] "));
        Serial.print(stateName(prev));
        Serial.print(F(" -> "));
        Serial.println(stateName(g_state));
    }
    // alarm/command/arm and alarm/command/disarm are intentionally ignored on the
    // kits. Node-RED publishes the updated alarm/system/state and kits follow it.
}

// ── Arduino entry points ──────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    for (uint32_t t = millis(); !Serial && (millis() - t) < 2000;) {}

    Serial.println(F("AlarmIoT  KIT_ID: " KIT_ID));

    feedbackBegin();
    displayBegin();
    displaySetState(SystemState::DISARMED, "Connecting...");
    displayTick();   // show splash immediately (before first loop())

    wifiBegin();
    mqttBegin();
    onMessage(onMqttMessage);
}

void loop() {
    wifiTick();
    mqttTick();
    feedbackTick();
    displayTick();
}
