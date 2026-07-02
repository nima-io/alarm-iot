#include "Mqtt.h"
#include "Network.h"
#include "../config/Topics.h"
#include "../config/Timings.h"
#include "secrets.h"

#include <Arduino.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <ArduinoJson.h>

// ── Internal state ────────────────────────────────────────────────────────────
static WiFiClient  _wifiClient;
static MqttClient  _mqtt(_wifiClient);

static void (*_userCb)(const char*, const char*) = nullptr;
static uint32_t    _lastRetry = 0;
static bool        _everConnected = false;

// ── Raw message handler (ArduinoMqttClient callback signature) ─────────────────
static void _onRawMessage(int messageSize) {
    String topic   = _mqtt.messageTopic();
    String payload = "";
    payload.reserve(static_cast<unsigned int>(messageSize));
    while (_mqtt.available()) {
        payload += static_cast<char>(_mqtt.read());
    }
    if (_userCb) {
        _userCb(topic.c_str(), payload.c_str());
    }
}

// ── Perform one connect attempt ───────────────────────────────────────────────
static void _doConnect() {
    Serial.print(F("[MQTT] Connecting to "));
    Serial.println(MQTT_HOST);

    // LWT — broker publishes "0" retained if this client disconnects uncleanly.
    _mqtt.beginWill(topicOnline(KIT_ID), /*retain=*/true, /*qos=*/0);
    _mqtt.print("0");
    _mqtt.endWill();

    // Credentials (only set when non-empty to avoid sending blank fields)
    if (sizeof(MQTT_USER) > 1) {   // sizeof of a string literal includes the NUL
        _mqtt.setUsernamePassword(MQTT_USER, MQTT_PASS);
    }

    if (!_mqtt.connect(MQTT_HOST, MQTT_PORT)) {
        Serial.print(F("[MQTT] Connect failed, error code: "));
        Serial.println(_mqtt.connectError());
        return;
    }

    Serial.println(F("[MQTT] Connected"));
    _everConnected = true;

    // Announce presence (retained "1" overrides the LWT "0")
    _mqtt.beginMessage(topicOnline(KIT_ID), /*retain=*/true, /*qos=*/0, /*dup=*/false);
    _mqtt.print("1");
    _mqtt.endMessage();

    // Subscribe to shared topics
    _mqtt.subscribe(TOPIC_SYSTEM_STATE);
    _mqtt.subscribe(TOPIC_CMD_ARM);
    _mqtt.subscribe(TOPIC_CMD_DISARM);
}

// ── Public API ────────────────────────────────────────────────────────────────
void mqttBegin() {
    _mqtt.onMessage(_onRawMessage);
    // Actual connection is deferred to mqttTick() so WiFi has time to come up.
}

void mqttTick() {
    if (!wifiConnected()) return;

    if (_mqtt.connected()) {
        _mqtt.poll();
        return;
    }

    // Reconnect with back-off
    const uint32_t now = millis();
    if (now - _lastRetry >= MQTT_RETRY_MS) {
        _lastRetry = now;
        _doConnect();
    }
}

void publishJson(const char* topic, JsonDocument& doc, bool retained) {
    if (!_mqtt.connected()) return;

    String json;
    serializeJson(doc, json);

    _mqtt.beginMessage(topic, retained, /*qos=*/0, /*dup=*/false);
    _mqtt.print(json);
    _mqtt.endMessage();
}

void onMessage(void (*cb)(const char* topic, const char* payload)) {
    _userCb = cb;
}
