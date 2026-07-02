# AlarmIoT — Implementation Plan

Two-kit networked alarm system: **Arduino MKR WiFi 1010 + Opla IoT Carrier** × 2, coordinated through **Mosquitto** (MQTT) and orchestrated/visualised by **Node-RED**. One PlatformIO project, one shared codebase, two build environments (`kit_a`, `kit_b`) selected at compile time via a `ROLE_*` flag. Node-RED owns the global state; kits are followers that publish events and react to the retained state topic.

This document is the end-of-project reference. Each phase below has explicit deliverables and a verification step. Complete a phase's verification before moving on.

---

## 1. Roles and responsibilities

| Component | Alias | Duties |
|---|---|---|
| Kit A | **Sentinel** | PIR motion detection, ambient telemetry, siren/LED feedback |
| Kit B | **Keypad** | 5-pad "defuse" ritual, ambient telemetry, siren/LED feedback |
| Mosquitto | **Broker** | MQTT transport; retained state; Last-Will/Testament |
| Node-RED | **Orchestrator** | Global FSM authority, dashboard, event log, presence tracking |

Node-RED is the single source of truth for `system_state`. Kits are stateless followers — this avoids split-brain if one kit reboots. Defuse validation is the one exception: it runs locally on Kit B because per-touch LED feedback needs zero-latency response.

---

## 2. State machine

Four states, owned by Node-RED, mirrored to kits via the retained `alarm/system/state` topic.

```
             cmd/arm (+ 10 s exit chirp on kits)
   DISARMED ─────────────────────────────────────► ARMED
       ▲                                            │
       │                                            │ motion event
       │                                            ▼
       │                                        PRE_ALARM
       │                                            │
       │                                            │ ENTRY_DELAY_MS elapses
       │                                            ▼
       │◄──────────── defused event ───────────  TRIGGERED
       │
       │◄──────────── cmd/disarm (admin override)
```

- `DISARMED` — idle. LEDs green (dim). No siren. Motion events ignored.
- `ARMED` — armed. LEDs blue. Motion events cause transition to `PRE_ALARM`.
- `PRE_ALARM` — grace period on Kit B. LEDs on Kit B all red (defuse target); Kit A LEDs amber, entry beep. Correct defuse → `DISARMED`; timer expiry → `TRIGGERED`.
- `TRIGGERED` — siren active on both kits. LEDs red-pulse. Defuse still valid → `DISARMED`. Admin `disarm` command from dashboard also valid.

The "arming exit delay" is not a separate state — it's a 10 s chirp inside the `DISARMED → ARMED` transition on the kits.

---

## 3. MQTT contract

Namespace: `alarm/<scope>/<name>`. State topics are **retained**; events and telemetry are not.

| Topic | Direction | Retained | Payload |
|---|---|---|---|
| `alarm/system/state` | NR → kits | yes | `{"state":"ARMED","since":<ms>}` |
| `alarm/kit_a/online` (LWT) | Kit A → all | yes | `1` on connect / `0` via LWT |
| `alarm/kit_b/online` (LWT) | Kit B → all | yes | `1` on connect / `0` via LWT |
| `alarm/kit_a/event/motion` | Kit A → NR | no | `{"ts":<ms>}` |
| `alarm/kit_b/event/defused` | Kit B → NR | no | `{"ts":<ms>}` |
| `alarm/kit_b/event/defuse_fail` | Kit B → NR | no | `{"ts":<ms>}` |
| `alarm/command/arm` | dashboard → NR → kits | no | `{}` |
| `alarm/command/disarm` | dashboard → NR → kits | no | `{}` (admin override; skips defuse) |
| `alarm/heartbeat/kit_a` | Kit A → NR | no | `{"rssi":-56,"uptime":123456}` (every 5 s) |
| `alarm/heartbeat/kit_b` | Kit B → NR | no | `{"rssi":-58,"uptime":123456}` (every 5 s) |
| `alarm/kit_a/telemetry` | Kit A → NR | no | `{"temp":22.4,"hum":41.0,"pres":1013.2,"lux":312}` (every 10 s) |
| `alarm/kit_b/telemetry` | Kit B → NR | no | same shape as Kit A (every 10 s) |

### Heartbeat vs. telemetry (what each is for)

- **Heartbeat** — proof-of-life ping every 5 s carrying `rssi` and `uptime`. LWT catches clean disconnects; heartbeat catches a firmware hang where the socket stays open but the loop has stopped. Node-RED marks a kit "stale" if two consecutive heartbeats are missed (~12 s).
- **Telemetry** — Opla carrier's ambient sensor readings (temperature, humidity, pressure, ambient light) published every 10 s for the dashboard to chart. Independent of the alarm logic; demonstrates the carrier's other sensors.

---

## 4. Defuse ritual (Kit B)

- The PIN is a **permutation of pads 1–5** (each pad used exactly once, length fixed at 5). Pad `N` maps to LED `N`.
- On entering `PRE_ALARM` or `TRIGGERED`, Kit B lights all 5 LEDs **red** and clears its progress counter.
- Correct next pad in sequence → that LED dims to off with a low beep.
- Wrong pad → all 5 LEDs flash white, high error beep, progress resets, `defuse_fail` event published.
- Fifth correct touch → `defused` event published; Node-RED transitions to `DISARMED`; both kits react to the retained state.
- On any transition into `DISARMED` or `ARMED`, the ritual is idle and LEDs follow the shared feedback module.

PIN lives on Kit B as a compile-time constant in `include/secrets.h` (`DEFUSE_PIN`) because per-touch LED feedback requires zero-latency local validation. Trade-off: changing the PIN requires re-flashing Kit B — acceptable for a school demo.

---

## 5. Repository layout (target)

```
AlarmIoT/
├── implementation_plan.md         ← this document
├── README.md                      ← quick-start (Phase 8)
├── .gitignore
├── platformio.ini                 ← two envs: kit_a, kit_b
├── include/
│   ├── secrets.example.h
│   └── secrets.h                  ← git-ignored
├── src/
│   ├── main.cpp                   ← role dispatch
│   ├── config/
│   │   ├── Topics.h
│   │   └── Timings.h
│   ├── core/
│   │   └── State.h
│   ├── net/
│   │   ├── Network.h / .cpp
│   │   └── Mqtt.h / .cpp
│   ├── ui/
│   │   ├── Feedback.h / .cpp      ← LED ring + buzzer
│   │   └── Display.h / .cpp       ← OLED
│   ├── sensors/
│   │   ├── Motion.h / .cpp        ← Kit A only (compiled everywhere; guarded in main)
│   │   └── Env.h / .cpp           ← telemetry (both kits)
│   ├── input/
│   │   └── Defuse.h / .cpp        ← Kit B only
│   └── telemetry/
│       └── Heartbeat.h / .cpp     ← both kits
├── nodered/
│   ├── flow.json
│   └── README.md
├── mosquitto/
│   ├── mosquitto.conf
│   └── README.md
└── docs/
    └── demo-script.md
```

---

## 6. Phased implementation

Each phase ends with a **Verification** step. Do not advance until it passes.

### Phase 0 — Prerequisites

1. Install PlatformIO extension in VS Code (already present since `platformio.ini` exists).
2. Install Mosquitto locally (Windows installer or `choco install mosquitto`).
3. Install Node.js LTS + Node-RED (`npm i -g --unsafe-perm node-red`) and the dashboard package (`npm i node-red-dashboard` inside `~/.node-red`).
4. Confirm both Opla kits enumerate as COM ports when plugged in.

**Verification:** `pio --version`, `mosquitto -h`, `node-red --version` all return successfully.

---

### Phase 1 — Repo scaffolding

**Deliverables**

- `platformio.ini` rewritten with a shared `[env]` block and two envs:
  - Common: `platform = atmelsam`, `board = mkrwifi1010`, `framework = arduino`, `monitor_speed = 115200`, `lib_deps` = `arduino-libraries/Arduino_MKRIoTCarrier`, `arduino-libraries/WiFiNINA`, `arduino-libraries/ArduinoMqttClient`, `bblanchon/ArduinoJson`.
  - `[env:kit_a]` → `build_flags = -DROLE_SENTINEL -DKIT_ID=\"kit_a\"`.
  - `[env:kit_b]` → `build_flags = -DROLE_KEYPAD -DKIT_ID=\"kit_b\"`.
- `include/secrets.example.h` with placeholders:
  - `WIFI_SSID`, `WIFI_PASS`
  - `MQTT_HOST`, `MQTT_PORT` (default `1883`), `MQTT_USER`, `MQTT_PASS`
  - `DEFUSE_PIN` (5-char string, permutation of `12345`, e.g. `"31524"`, Kit B only — safe to leave defined on Kit A build, just unused)
- `.gitignore` — PlatformIO defaults (`.pio/`, `.pioenvs/`, `.piolibdeps/`) plus `include/secrets.h`, `.vscode/`, `nodered/.node-red/`.
- Placeholder `src/main.cpp` that only prints `KIT_ID` to serial in `setup()`.

**Verification:** `pio run -e kit_a` and `pio run -e kit_b` both compile. Uploading each to its kit shows the correct `KIT_ID` on the serial monitor.

---

### Phase 2 — Network foundation

**Deliverables**

- `src/config/Topics.h` — all topic strings from §3 as `constexpr const char*`, with helpers `topicOnline(KIT_ID)`, `topicHeartbeat(KIT_ID)`, `topicTelemetry(KIT_ID)`.
- `src/config/Timings.h` — `EXIT_DELAY_MS = 10000`, `ENTRY_DELAY_MS = 15000`, `HEARTBEAT_MS = 5000`, `TELEMETRY_MS = 10000`, `PIR_DEBOUNCE_MS = 1500`, `WIFI_RETRY_MS = 5000`, `MQTT_RETRY_MS = 3000`.
- `src/net/Network.h/.cpp` — `void wifiBegin();` and `void wifiTick();`. Non-blocking reconnect using `millis()` and `WIFI_RETRY_MS`. Exposes `bool wifiConnected()`.
- `src/net/Mqtt.h/.cpp`:
  - `void mqttBegin();` sets LWT on `alarm/<KIT_ID>/online` payload `0` retained.
  - `void mqttTick();` reconnects on disconnect; on successful connect publishes `1` retained to the same topic and subscribes to `alarm/system/state`, `alarm/command/arm`, `alarm/command/disarm`.
  - `void publishJson(const char* topic, JsonDocument& doc, bool retained = false);` helper.
  - `void onMessage(void (*cb)(const char* topic, const char* payload));` callback registration.
- `src/main.cpp` updated to `wifiBegin(); mqttBegin();` in `setup()` and `wifiTick(); mqttTick();` in `loop()`. No `delay()`.

**Verification:** Start Mosquitto locally. Run `mosquitto_sub -h <host> -t "alarm/#" -v`. Boot each kit; each should publish `alarm/kit_x/online 1` retained. Unplug a kit; within a few seconds the topic should show `0` (LWT).

---

### Phase 3 — FSM + shared UI

**Deliverables**

- `src/core/State.h` — `enum class SystemState { DISARMED, ARMED, PRE_ALARM, TRIGGERED };` plus `const char* stateName(SystemState);` and `SystemState stateFromString(const char*);`.
- `src/ui/Feedback.h/.cpp`:
  - `void feedbackBegin();` initialises carrier LEDs and buzzer.
  - `void feedbackApplyState(SystemState s);` sets the LED ring colour and idle buzzer pattern per state (green / blue / amber-pulse / red-pulse). Defuse's per-LED overrides on Kit B happen through explicit `feedbackSetLed(index, r, g, b)` calls so `Defuse.cpp` can drive individual LEDs during `PRE_ALARM`/`TRIGGERED`.
  - `void feedbackTick();` runs the pulse animation and the siren cycle.
  - `void feedbackChirpArming();` — 10 s exit-delay chirp scheduled by `main.cpp` on the `DISARMED → ARMED` transition.
- `src/ui/Display.h/.cpp`:
  - `void displayBegin();`
  - `void displaySetState(SystemState s, const char* subtitle);` renders kit id + state + subtitle (used for countdowns, "Enter code", "Wrong!" messages, etc.).
  - `void displayTick();` refreshes only when dirty to avoid flicker.
- `src/main.cpp`:
  - Cache current `SystemState state = DISARMED;`.
  - Register MQTT callback: on `alarm/system/state` payload, parse to enum, update `state`, call `feedbackApplyState` and `displaySetState`, and dispatch role-specific side effects (below).
  - Handle `alarm/command/arm` and `alarm/command/disarm` by ignoring locally — Node-RED will publish the new `alarm/system/state` and kits react to that. This keeps the kits stateless.

**Verification:** With `mosquitto_pub`, publish retained `alarm/system/state` payloads `{"state":"DISARMED"}`, `{"state":"ARMED"}`, `{"state":"PRE_ALARM"}`, `{"state":"TRIGGERED"}` in turn. Both kits should update their LED colour, buzzer, and OLED accordingly within ~1 s.

---

### Phase 4 — Sentinel role (Kit A)

**Deliverables**

- `src/sensors/Motion.h/.cpp`:
  - `void motionBegin();` initialises PIR input.
  - `bool motionTick();` returns `true` at most once per `PIR_DEBOUNCE_MS` when motion is detected.
- `src/main.cpp` under `#ifdef ROLE_SENTINEL`:
  - Call `motionBegin()` in `setup()`.
  - In `loop()`, when `motionTick()` and `state == ARMED`, publish `alarm/kit_a/event/motion` with `{"ts":millis()}`.
  - Ignore motion events in other states.

**Verification:** Publish `alarm/system/state {"state":"ARMED"}` retained. Wave a hand in front of Kit A's PIR. `mosquitto_sub -t "alarm/kit_a/event/motion" -v` prints an event. Repeat within 1 s — debounced, no duplicate.

---

### Phase 5 — Keypad role (Kit B)

**Deliverables**

- `src/input/Defuse.h/.cpp`:
  - `void defuseBegin(const char* pin);` stores the target PIN (from `DEFUSE_PIN`) and initialises touch reading.
  - `void defuseArm();` called when state becomes `PRE_ALARM` or `TRIGGERED`: sets all 5 LEDs red via `feedbackSetLed`, resets progress index to 0.
  - `void defuseDisarm();` called when state becomes `DISARMED` or `ARMED`: releases LED control back to `feedbackApplyState`.
  - `void defuseTick();`: poll all 5 pads with edge-detection + debounce; on a press, compare to `pin[progress]`. Correct → dim LED `pad`, low beep, increment progress; on `progress == 5`, publish `alarm/kit_b/event/defused` and enter idle. Wrong → flash all 5 LEDs white for ~300 ms, high beep, publish `alarm/kit_b/event/defuse_fail`, reset progress to 0, re-light all red.
- `src/main.cpp` under `#ifdef ROLE_KEYPAD`:
  - `defuseBegin(DEFUSE_PIN)` in `setup()`.
  - In the state-change callback, call `defuseArm()` on entering `PRE_ALARM`/`TRIGGERED`, `defuseDisarm()` on entering `DISARMED`/`ARMED`.
  - `defuseTick()` in `loop()`.

**Verification:** With `alarm/system/state` set to `TRIGGERED`, all 5 LEDs on Kit B glow red. Tapping the correct sequence extinguishes each LED and, on the 5th, `alarm/kit_b/event/defused` fires. Any wrong pad restarts the ritual and fires `alarm/kit_b/event/defuse_fail`.

---

### Phase 6 — Heartbeat + telemetry

**Deliverables**

- `src/telemetry/Heartbeat.h/.cpp`:
  - `void heartbeatBegin();`
  - `void heartbeatTick();` every `HEARTBEAT_MS`, publishes `alarm/heartbeat/<KIT_ID>` with `{"rssi":WiFi.RSSI(),"uptime":millis()}`.
- `src/sensors/Env.h/.cpp`:
  - `void envBegin();` initialises the carrier's environmental sensors.
  - `void envTick();` every `TELEMETRY_MS`, reads temperature, humidity, pressure, and ambient light and publishes `alarm/<KIT_ID>/telemetry` with `{"temp":..,"hum":..,"pres":..,"lux":..}`.
- `src/main.cpp`:
  - `heartbeatBegin(); envBegin();` in `setup()`.
  - `heartbeatTick(); envTick();` in `loop()` — compiled for both roles.

**Verification:** `mosquitto_sub -t "alarm/heartbeat/#" -v` shows a payload from each kit every ~5 s. `mosquitto_sub -t "alarm/+/telemetry" -v` shows sensor payloads every ~10 s from each kit.

---

### Phase 7 — Node-RED flow + dashboard

**Deliverables** — `nodered/flow.json` importable into Node-RED, containing:

- **MQTT-broker config node** pointing at the same broker as the kits.
- **MQTT-in nodes** for:
  - `alarm/kit_a/event/motion`
  - `alarm/kit_b/event/defused`
  - `alarm/kit_b/event/defuse_fail`
  - `alarm/+/online`
  - `alarm/heartbeat/+`
  - `alarm/+/telemetry`
  - `alarm/command/arm`
  - `alarm/command/disarm`
- **FSM function node** holding current state in `flow.state` (default `DISARMED`). Transitions:
  - `cmd/arm` while `DISARMED` → `ARMED`.
  - `cmd/disarm` (any state) → `DISARMED`.
  - `event/motion` while `ARMED` → `PRE_ALARM`, start `ENTRY_DELAY_MS` timer.
  - Timer expiry in `PRE_ALARM` → `TRIGGERED`.
  - `event/defused` while `PRE_ALARM`/`TRIGGERED` → `DISARMED`.
  - `event/defuse_fail` → no state change; log entry.
- **MQTT-out node** publishing every state change to `alarm/system/state` **retained** with `{"state":..,"since":..}`.
- **Presence tracker** — function node holds last heartbeat timestamp per kit; on each heartbeat, updates a Dashboard indicator. If `now - last > 12000 ms` for a kit, its indicator turns red.
- **Dashboard** (`ui_group`s under an "AlarmIoT" tab):
  - State badge (large text with colour matching kit LED colour).
  - Buttons: **Arm**, **Disarm** (admin override), **Silence** optional.
  - Kit A + Kit B online indicators (from LWT `online` topic).
  - Kit A + Kit B "live" indicators (from heartbeat freshness).
  - Line charts for each telemetry field per kit (temp, humidity, pressure, lux) — last 30 min.
  - Scrolling event log (motion, defused, defuse_fail, arm/disarm commands).
- `nodered/README.md` — one paragraph: how to import `flow.json`, install `node-red-dashboard`, and where to configure the broker credentials.

**Verification:** Import the flow, hit **Deploy**. From the dashboard, click **Arm** — both kits go blue and OLEDs show `ARMED`. Wave at Kit A — state badge goes amber (`PRE_ALARM`), Kit B lights all red, event log shows the motion. Enter the correct sequence on Kit B — badge goes green (`DISARMED`), siren stops. Wait > 12 s with a kit unplugged — its "live" indicator goes red.

---

### Phase 8 — Broker config + docs

**Deliverables**

- `mosquitto/mosquitto.conf`:
  - `listener 1883`
  - `allow_anonymous false`
  - `password_file <path>/passwd`
- `mosquitto/README.md` — the two commands: `mosquitto_passwd -c passwd alarmiot` to create the user, and `mosquitto -c mosquitto.conf -v` to run the broker.
- Root `README.md` covering:
  - Parts list (2× MKR WiFi 1010, 2× Opla carrier, host PC for broker/Node-RED).
  - Setup: fill `include/secrets.h` (copy from example), set the same values in the Node-RED MQTT config.
  - Build/upload: `pio run -e kit_a -t upload`, `pio run -e kit_b -t upload`.
  - Run: start Mosquitto, start Node-RED, open `http://localhost:1880/ui`.
  - The demo script (below).
  - Known limitations: plaintext MQTT (LAN only), PIN in firmware, no persistence across NR restart unless context storage enabled.
- `docs/demo-script.md` — the 90-second demo flow used at presentation:
  1. Show both kits online on dashboard.
  2. Click **Arm**, show 10 s exit chirp and blue LEDs.
  3. Wave at Kit A, show `PRE_ALARM` on both kits and Kit B lighting red.
  4. Type one wrong pad → white flash + reset.
  5. Type correct sequence → LEDs go out one by one → `DISARMED` on both kits.
  6. Point at temperature chart to show telemetry is live.

**Verification:** A fresh user can clone the repo, follow only `README.md`, and reach a working demo.

---

### Phase 9 — Integration test / demo dry-run

**Deliverables** — an executed dry-run following `docs/demo-script.md` end-to-end at least twice, on the actual hardware. Note any timing issues (LED debounce feel, buzzer volume, PIR sensitivity) and tune constants in `src/config/Timings.h` accordingly.

**Verification:** Both dry-runs complete without a reset or manual MQTT intervention.

---

## 7. Design decisions (locked in)

- **State authority = Node-RED.** Simpler than distributed consensus; kits recover on reboot via retained state.
- **Defuse validation local to Kit B.** Per-touch LED feedback demands zero latency; the PIN must live on the device. Node-RED only observes `defused` / `defuse_fail`.
- **PIN as compile-time constant in `secrets.h`.** Documented limitation; changing it requires a re-flash of Kit B.
- **Role at compile time via build flags.** Each kit gets a dedicated firmware image; no runtime role selection to debug.
- **Four states only.** Exit delay is a UI chirp, not a state. Keeps the FSM readable.
- **Heartbeat + telemetry included.** Heartbeat guards against firmware hangs (LWT does not). Telemetry showcases the carrier's other sensors on the dashboard.
- **Plaintext MQTT over LAN.** Acceptable for a school demo on an isolated network; listed as a limitation in the README.

## 8. Timing constants (initial values, tune in Phase 9)

| Constant | Value | Where |
|---|---|---|
| `EXIT_DELAY_MS` | 10 000 | Kits (arming chirp) |
| `ENTRY_DELAY_MS` | 15 000 | Node-RED (PRE_ALARM → TRIGGERED) |
| `HEARTBEAT_MS` | 5 000 | Kits |
| `TELEMETRY_MS` | 10 000 | Kits |
| `PIR_DEBOUNCE_MS` | 1 500 | Kit A |
| `WIFI_RETRY_MS` | 5 000 | Kits |
| `MQTT_RETRY_MS` | 3 000 | Kits |
| Presence stale threshold | 12 000 | Node-RED |

## 9. Handoff checklist

Before declaring done, confirm:

- [ ] All nine phases' verification steps pass on real hardware.
- [ ] `pio run -e kit_a` and `pio run -e kit_b` both compile without warnings.
- [ ] `include/secrets.h` is git-ignored; only the example is committed.
- [ ] `nodered/flow.json` re-imports cleanly on a fresh Node-RED install.
- [ ] `README.md` demo script works for someone who did not build the project.
- [ ] `docs/demo-script.md` matches the actual presentation flow.
