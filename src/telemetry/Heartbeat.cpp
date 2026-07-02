#include "Heartbeat.h"
#include "../config/Timings.h"
#include "../config/Topics.h"
#include "../net/Mqtt.h"

#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoJson.h>

static uint32_t _lastHb = 0;

void heartbeatBegin() {
    _lastHb = millis();
}

void heartbeatTick() {
    const uint32_t now = millis();
    if (now - _lastHb < HEARTBEAT_MS) return;
    _lastHb = now;

    JsonDocument doc;
    doc["rssi"]   = WiFi.RSSI();
    doc["uptime"] = now;
    publishJson(topicHeartbeat(KIT_ID), doc);
}
