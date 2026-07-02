#include <Arduino.h>
#include "net/Network.h"
#include "net/Mqtt.h"

void setup() {
    Serial.begin(115200);
    // Wait up to 2 s for a serial console; skip if none is attached.
    for (uint32_t t = millis(); !Serial && (millis() - t) < 2000;) {}

    Serial.println("KIT_ID: " KIT_ID);

    wifiBegin();
    mqttBegin();
}

void loop() {
    wifiTick();
    mqttTick();
}
