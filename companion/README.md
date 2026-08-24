# AeroGuard companion

Phone-first web app for the contest pitch: **Mi Home–style device home** with **light/dark liquid-glass** UI.

## Connectivity story

| Layer | Role |
|-------|------|
| **ESP32 WiFi** | Remote status + vent commands on the home network (`esp32_aeroguard_bridge.ino`) |
| **GSM** | SMS / call when you are away (hardware) |
| **Contest UI** | Discovery / live link can still be simulated until the app points at the ESP32 `/status` URL |

## What you get

- Home with stats, scenes, and 2-column device cards
- Pair → first-time setup → device hub
- Per device: **Status**, **Activity**, **Vents**, **Device**
- Theme toggle (persisted)

```bash
cd companion
npm install
npm run dev -- --hostname 127.0.0.1 --port 43123
```

Open [http://127.0.0.1:43123](http://127.0.0.1:43123) on a phone (same Wi‑Fi) or Add to Home Screen.
