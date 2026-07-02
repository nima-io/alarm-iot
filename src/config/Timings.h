#pragma once

#include <stdint.h>

constexpr uint32_t EXIT_DELAY_MS    = 10000;  // arming exit-chirp window (ms)
constexpr uint32_t ENTRY_DELAY_MS   = 15000;  // PRE_ALARM → TRIGGERED delay (ms, Node-RED)
constexpr uint32_t HEARTBEAT_MS     =  5000;  // proof-of-life publish interval (ms)
constexpr uint32_t TELEMETRY_MS     = 10000;  // ambient sensor publish interval (ms)
constexpr uint32_t PIR_DEBOUNCE_MS  =  1500;  // minimum gap between motion events (ms)
constexpr uint32_t WIFI_RETRY_MS    =  5000;  // WiFi reconnect back-off (ms)
constexpr uint32_t MQTT_RETRY_MS    =  3000;  // MQTT reconnect back-off (ms)
