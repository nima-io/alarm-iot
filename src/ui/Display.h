#pragma once

#include "../core/State.h"

// Prepare the display for use.
// Must be called after feedbackBegin() (which calls carrier.begin()).
void displayBegin();

// Schedule a display update showing the kit ID, state name, and an optional
// subtitle (e.g. "Enter code!", "Wrong!", a countdown string).
// The subtitle string is copied internally, so the caller's buffer may be
// freed or reused immediately after this call returns.
void displaySetState(SystemState s, const char* subtitle);

// Flush any pending display update.
// The render is deferred to this tick to avoid blocking the MQTT callback.
void displayTick();
