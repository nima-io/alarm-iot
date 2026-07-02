#pragma once

#include <ArduinoJson.h>

// Configure LWT and register the internal message handler.
// Must be called once in setup(), after wifiBegin().
void mqttBegin();

// Non-blocking reconnect + poll tick — call every loop iteration.
// Re-publishes the online flag and re-subscribes on each reconnect.
void mqttTick();

// Serialise doc to JSON and publish to topic.
// retained = true publishes with the MQTT retain flag set.
void publishJson(const char* topic, JsonDocument& doc, bool retained = false);

// Register a callback invoked for every subscribed message.
// The callback receives the topic string and a null-terminated payload string.
// Pass nullptr to deregister.
void onMessage(void (*cb)(const char* topic, const char* payload));
