#pragma once

// Copy this file to secrets.h and fill in your values.
// secrets.h is git-ignored — never commit it.

#define WIFI_SSID   "your_wifi_ssid"
#define WIFI_PASS   "your_wifi_password"

#define MQTT_HOST   "192.168.x.x"
#define MQTT_PORT   1883
#define MQTT_USER   ""
#define MQTT_PASS   ""

// 5-character PIN: each digit 1-5 used exactly once (permutation of "12345")
// Used by Kit B only; defined here so both builds compile cleanly.
#define DEFUSE_PIN  "31524"
