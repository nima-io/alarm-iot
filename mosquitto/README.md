# Mosquitto broker

`mosquitto.conf` is already configured for password-authenticated MQTT on port 1883.
The password file is **not committed** to the repo — create it once before the first
`compose up`:

```bash
# Run from the repo root — creates mosquitto/passwd with username "alarmiot"
docker compose run --rm mosquitto \
  mosquitto_passwd -c /mosquitto/config/passwd alarmiot
```

You will be prompted for a password. Use the same value you set as `MQTT_PASS` in
`include/secrets.h` and in the Node-RED MQTT broker config.

## Daily use

| Action | Command |
|---|---|
| Start all services | `docker compose up -d` |
| Stop all services | `docker compose down` |
| View broker logs | `docker compose logs -f mosquitto` |
| View Node-RED logs | `docker compose logs -f nodered` |
