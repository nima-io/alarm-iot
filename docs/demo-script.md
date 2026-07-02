# AlarmIoT - 90-second demo script

Prerequisite: both kits powered, docker compose up -d running, dashboard open at
http://localhost:1880/ui

---

## Step 1 - Show both kits online (~10 s)

Point at the Kit Status panel.
Both Kit A and Kit B rows show ONLINE and LIVE.
State badge shows DISARMED (green).

"Both kits are connected to the broker. LWT and heartbeat confirm they are alive."

---

## Step 2 - Arm the system (~15 s)

Click ARM on the dashboard.

- State badge turns blue: ARMED
- Both kit OLEDs show ARMED
- Both kits emit one chirp per second for 10 s (arming exit delay)
- LEDs on both kits turn solid blue

"The dashboard button published an arm command. Node-RED transitioned to ARMED
and pushed the retained state to both kits. The 10-second exit chirp gives time
to leave the area."

---

## Step 3 - Trigger motion (~10 s)

Wave a hand in front of Kit A PIR sensor.

- State badge turns orange: PRE_ALARM
- Kit A OLED shows PRE_ALARM, LEDs pulse amber
- Kit B OLED shows PRE_ALARM / Enter code!, all 5 LEDs light red
- Event log: Motion detected (Kit A)

"Kit A detected motion and published an event. Node-RED moved to PRE_ALARM and
started the 15-second entry-delay timer. Kit B waits for the defuse sequence."

---

## Step 4 - Enter a wrong pad (~5 s)

On Kit B, tap any pad that is NOT the first digit of the PIN.

- All 5 LEDs flash white for ~300 ms, then return to red
- High-pitched error beep plays
- Event log: Defuse FAILED

"Wrong pad. The ritual restarts from zero."

---

## Step 5 - Enter the correct sequence (~15 s)

Tap the 5 pads in PIN order (e.g. 3 then 1 then 5 then 2 then 4 for PIN 31524).

- Each correct tap: that LED extinguishes, short low beep
- On the 5th correct tap: all LEDs off
- State badge turns green: DISARMED
- Siren stops on both kits
- Event log: DEFUSED!

"Correct sequence entered. Kit B published a defused event, Node-RED transitioned
to DISARMED, and both kits reacted to the retained state update."

---

## Step 6 - Show live telemetry (~10 s)

Point at the Telemetry panel.

- Temperature, humidity, pressure, and ambient light charts show two series:
  Kit A and Kit B, updated every 10 seconds
- Last 30 minutes of readings are retained

"Both Opla Carriers publish continuous ambient sensor data independent of the
alarm logic, demonstrating the full carrier sensor suite."

---

## Bonus - staleness indicator (~15 s, optional)

Unplug one kit. After ~12 seconds its live indicator changes from LIVE to STALE.
Plug it back in - within one heartbeat interval (<=5 s) it returns to LIVE.

"The LWT marks the kit offline instantly. The heartbeat freshness check catches
firmware hangs where the socket stays open but the loop has stopped."
