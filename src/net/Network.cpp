#include "Network.h"
#include "../config/Timings.h"
#include "secrets.h"

#include <Arduino.h>
#include <WiFiNINA.h>

static uint32_t _lastRetry = 0;
static bool     _beganOnce = false;

void wifiBegin() {
    _beganOnce = true;
    Serial.print(F("[WiFi] Connecting to "));
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void wifiTick() {
    if (WiFi.status() == WL_CONNECTED) return;

    const uint32_t now = millis();
    if (now - _lastRetry >= WIFI_RETRY_MS) {
        _lastRetry = now;
        Serial.println(F("[WiFi] Reconnecting..."));
        if (_beganOnce) {
            WiFi.end();
        }
        WiFi.begin(WIFI_SSID, WIFI_PASS);
        _beganOnce = true;
    }
}

bool wifiConnected() {
    return WiFi.status() == WL_CONNECTED;
}
