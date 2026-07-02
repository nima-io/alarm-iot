#pragma once

// Initialise the PIR sensor pin as an input.
void motionBegin();

// Non-blocking motion tick — call every loop iteration.
// Returns true at most once per PIR_DEBOUNCE_MS when a rising edge is detected
// on the PIR output. Returns false for all subsequent calls until the debounce
// window has elapsed.
bool motionTick();
