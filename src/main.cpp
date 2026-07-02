#include <Arduino.h>

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("KIT_ID: " KIT_ID);
}

void loop() {
}
