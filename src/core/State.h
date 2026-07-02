#pragma once

#include <Arduino.h>

enum class SystemState {
    DISARMED  = 0,
    ARMED     = 1,
    PRE_ALARM = 2,
    TRIGGERED = 3,
};

inline const char* stateName(SystemState s) {
    switch (s) {
        case SystemState::DISARMED:  return "DISARMED";
        case SystemState::ARMED:     return "ARMED";
        case SystemState::PRE_ALARM: return "PRE_ALARM";
        case SystemState::TRIGGERED: return "TRIGGERED";
        default:                     return "UNKNOWN";
    }
}

// Parses the "state" string from the alarm/system/state JSON payload.
// Returns DISARMED for any unrecognised string.
inline SystemState stateFromString(const char* s) {
    if (s == nullptr)              return SystemState::DISARMED;
    if (strcmp(s, "ARMED")     == 0) return SystemState::ARMED;
    if (strcmp(s, "PRE_ALARM") == 0) return SystemState::PRE_ALARM;
    if (strcmp(s, "TRIGGERED") == 0) return SystemState::TRIGGERED;
    return SystemState::DISARMED;
}
