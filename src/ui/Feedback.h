#pragma once

#include "../core/State.h"
#include <Arduino_MKRIoTCarrier.h>

// The carrier object is defined in Feedback.cpp; Display.cpp uses it via extern.
extern MKRIoTCarrier carrier;

// Initialise the carrier (calls carrier.begin()) and set the initial LED state.
// Must be called before displayBegin() or any other carrier use.
void feedbackBegin();

// Apply the LED ring colour and buzzer idle pattern for the new state.
// Stops any in-progress siren or chirp before re-configuring.
void feedbackApplyState(SystemState s);

// Non-blocking animation/buzzer tick — call every loop iteration.
// Drives LED pulse animations and the siren/chirp state machines.
void feedbackTick();

// Start the 10 s arming exit-delay chirp.
// Call once from main.cpp when the state transitions DISARMED → ARMED.
void feedbackChirpArming();

// Directly set one LED pixel and push the update.
// Used by the Defuse module (Phase 5) to take per-LED control during
// PRE_ALARM and TRIGGERED on Kit B without interfering with the base
// animations on Kit A.
void feedbackSetLed(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
