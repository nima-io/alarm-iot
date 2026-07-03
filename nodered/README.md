# Node-RED flow

## One-time setup

1. Install the **@flowfuse/node-red-dashboard** palette inside Node-RED:
   open `http://localhost:1880` → **Menu ☰ → Manage Palette → Install** → search `@flowfuse/node-red-dashboard` → install.

2. Import the flow:
   **Menu ☰ → Import → select file** → choose `nodered/flow.json` → **Import**.

3. Configure MQTT credentials:
   double-click any MQTT node → click the pencil icon next to the broker
   → set **Username** and **Password** to match the values you put in
   `include/secrets.h` (`MQTT_USER` / `MQTT_PASS`) and the broker's `passwd` file.
   The **Server** field must be `mosquitto` (the Docker Compose service name), not `localhost`.

4. Click **Deploy**.

## Dashboard

Open `http://localhost:1880/ui` to reach the AlarmIoT dashboard tab.

## Stopping / restarting

```
docker compose down        # stop all services
docker compose up -d       # start (or restart) all services
docker compose logs -f nodered     # follow Node-RED logs
```

> **Note:** flow context (`flow.state`) is stored in memory and is lost on Node-RED
> restart. To persist it across restarts, enable file-based context storage in the
> Node-RED `settings.js` (see the Node-RED docs on *Context Storage*).
