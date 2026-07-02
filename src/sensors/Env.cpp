#include "Env.h"
#include "../config/Timings.h"
#include "../config/Topics.h"
#include "../net/Mqtt.h"
#include "../ui/Feedback.h"   // extern MKRIoTCarrier carrier

#include <Arduino.h>
#include <ArduinoJson.h>

static uint32_t _lastTelemetry = 0;

void envBegin() {
    _lastTelemetry = millis();
    // All carrier sensors are already initialised by carrier.begin()
    // inside feedbackBegin() — no additional setup needed here.
}

void envTick() {
    const uint32_t now = millis();
    if (now - _lastTelemetry < TELEMETRY_MS) return;
    _lastTelemetry = now;

    JsonDocument doc;
    doc["temp"] = carrier.Env.readTemperature();
    doc["hum"]  = carrier.Env.readHumidity();
    doc["pres"] = carrier.Pressure.readPressure();

    // Ambient light: sum R+G+B channels from the APDS9960 as a lux proxy.
    // The clear channel integration may not be ready on every tick; default to 0.
    int lux = 0;
    if (carrier.Light.colorAvailable()) {
        int r, g, b;
        carrier.Light.readColor(r, g, b);
        lux = r + g + b;
    }
    doc["lux"] = lux;

    publishJson(topicTelemetry(KIT_ID), doc);
}
