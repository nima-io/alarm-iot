#pragma once

// Store the target PIN (from DEFUSE_PIN) and prepare the touch subsystem.
// Must be called once in setup(), after feedbackBegin().
void defuseBegin(const char* pin);

// Enter defuse-active mode: light all 5 LEDs red and reset the progress counter.
// Call when system state becomes PRE_ALARM or TRIGGERED.
void defuseArm();

// Exit defuse mode and release LED control back to feedbackApplyState().
// Call when system state becomes DISARMED or ARMED.
void defuseDisarm();

// Non-blocking pad-polling tick — call every loop iteration.
// Correct pad in sequence → dim its LED + low beep, advance progress.
// Fifth correct pad     → publish defused event, enter idle.
// Wrong pad             → white flash (~300 ms) + high beep, publish defuse_fail,
//                         reset progress to 0, re-light all red.
void defuseTick();
