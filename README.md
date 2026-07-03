# AlarmIoT

Two-kit networked alarm system built on **Arduino MKR WiFi 1010 + Opla IoT Carrier**,
coordinated through **Mosquitto** (MQTT) and orchestrated/visualised by **Node-RED**.

## Parts list

| Qty | Item |
|-----|------|
| 2 | Arduino MKR WiFi 1010 |
| 2 | Arduino Opla IoT Carrier |
| 1 | Host PC (runs Mosquitto + Node-RED via Docker) |
| 1 | HC-SR501 PIR sensor (wired to Kit A pin D6) |
| — | USB cables, LAN / Wi-Fi access point |

## Quick-start

### 1 — Fill in secrets

```bash
cp include/secrets.example.h include/secrets.h
```

Edit `include/secrets.h` and set:

| Field | Description |
|---|---|
| `WIFI_SSID` / `WIFI_PASS` | Your Wi-Fi network credentials |
| `MQTT_HOST` | LAN IP of the host PC running Docker |
| `MQTT_PORT` | `1883` (default) |
| `MQTT_USER` / `MQTT_PASS` | Broker credentials (same as `passwd` file) |
| `DEFUSE_PIN` | 5-digit permutation of `12345`, e.g. `"31524"` |

### 2 — Create the broker password file (once)

```bash
docker compose run --rm mosquitto \
  mosquitto_passwd -c /mosquitto/config/passwd alarmiot

sudo chmod 644 mosquitto/passwd
```

Enter the same password you put in `MQTT_PASS`.

> **Note:** `docker compose run` creates the file as `root` (mode `600`).
> The `chmod 644` makes it readable by the `mosquitto` container user (UID 1883).
> The file stores only password hashes, so world-readable is safe.

### 3 — Start broker + Node-RED

```bash
docker compose up -d
```

### 4 — Set up the Node-RED dashboard

1. Open `http://localhost:1880`.
2. **Menu ☰ → Manage Palette → Install** → search `@flowfuse/node-red-dashboard` → install.
3. **Menu ☰ → Import → select file** → choose `nodered/flow.json` → **Import**.
4. Open the **Mosquitto** broker config node, enter `MQTT_USER` / `MQTT_PASS`, click **Done**.
5. Click **Deploy**.
6. Open `http://localhost:1880/ui` — the AlarmIoT dashboard should appear.

### 5 — Build and upload firmware

```bash
# Kit A (Sentinel — PIR sensor)
pio run -e kit_a -t upload

# Kit B (Keypad — defuse ritual)
pio run -e kit_b -t upload
```

Both kits should appear as **ONLINE** on the dashboard within a few seconds.

## Demo

See [`docs/demo-script.md`](docs/demo-script.md) for the full 90-second presentation script.

**One-liner summary:**
1. Dashboard shows both kits online.
2. Click **ARM** — kits chirp for 10 s, LEDs turn blue.
3. Wave at Kit A's PIR — state goes **PRE_ALARM**, Kit B LEDs light red.
4. Enter wrong pad on Kit B → white flash.
5. Enter correct 5-pad sequence → **DISARMED**, siren stops.
6. Point at telemetry charts — live sensor data from both kits.

## Known limitations

- **Plaintext MQTT** — suitable for an isolated LAN demo only; no TLS.
- **PIN in firmware** — changing `DEFUSE_PIN` requires re-flashing Kit B.
- **No FSM persistence** — Node-RED restarts as `DISARMED`. Enable file-based
  context storage in Node-RED `settings.js` to survive restarts.
