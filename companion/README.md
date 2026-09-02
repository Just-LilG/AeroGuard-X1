# AeroGuard companion

Phone-first web app for the contest pitch: **Mi Home–style device home** with **light/dark liquid-glass** UI.

## Connectivity story

| Layer | Role |
|-------|------|
| **Contest UI** | Simulated discovery and status (this Vercel app does not talk to the box) |
| **GSM** | Real SMS / call from the SIM800L on the Arduino Uno |
| **ESP32 WiFi** | Later / optional — not part of the demo kit |

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
