#pragma once

// Initialise the telemetry timer. Call once in setup(), after feedbackBegin().
void envBegin();

// Non-blocking telemetry tick — call every loop iteration.
// Every TELEMETRY_MS reads temperature, humidity, pressure, and ambient light
// from the Opla Carrier and publishes alarm/<KIT_ID>/telemetry with
// {"temp":.., "hum":.., "pres":.., "lux":..}.
void envTick();
