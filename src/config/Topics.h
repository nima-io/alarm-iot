#pragma once

// ── Fixed topics (not kit-specific) ──────────────────────────────────────────
constexpr const char* TOPIC_SYSTEM_STATE       = "alarm/system/state";
constexpr const char* TOPIC_CMD_ARM            = "alarm/command/arm";
constexpr const char* TOPIC_CMD_DISARM         = "alarm/command/disarm";

// Kit-specific event topics (used by the relevant role only)
constexpr const char* TOPIC_KIT_A_MOTION       = "alarm/kit_a/event/motion";
constexpr const char* TOPIC_KIT_B_DEFUSED      = "alarm/kit_b/event/defused";
constexpr const char* TOPIC_KIT_B_DEFUSE_FAIL  = "alarm/kit_b/event/defuse_fail";

// ── Kit-specific topic helpers ─────────────────────────────────────────────────
// These macros use compile-time string literal concatenation with the KIT_ID
// build flag so each binary embeds its own topic strings with zero overhead.
//
//   topicOnline(KIT_ID)    → "alarm/kit_a/online"
//   topicHeartbeat(KIT_ID) → "alarm/heartbeat/kit_a"
//   topicTelemetry(KIT_ID) → "alarm/kit_a/telemetry"
#define topicOnline(kit)     "alarm/" kit "/online"
#define topicHeartbeat(kit)  "alarm/heartbeat/" kit
#define topicTelemetry(kit)  "alarm/" kit "/telemetry"
