#pragma once

// Initialise WiFi and begin connecting.
void wifiBegin();

// Non-blocking reconnect tick — call every loop iteration.
// Attempts reconnection at most once per WIFI_RETRY_MS when disconnected.
void wifiTick();

// Returns true when the WiFi link is up.
bool wifiConnected();
