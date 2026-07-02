#pragma once

// Initialise the heartbeat timer. Call once in setup().
void heartbeatBegin();

// Non-blocking heartbeat tick — call every loop iteration.
// Every HEARTBEAT_MS publishes alarm/heartbeat/<KIT_ID> with
// {"rssi":<dBm>,"uptime":<ms>} so Node-RED can detect firmware hangs
// that the LWT alone would miss.
void heartbeatTick();
