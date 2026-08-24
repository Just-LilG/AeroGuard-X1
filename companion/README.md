# AeroGuard companion

Phone-first web app for the contest pitch: **Mi Home–style device home** with **light/dark liquid-glass** UI (iOS-inspired glass panels).

## What you get

- Home with stats, scenes, and 2-column device cards
- Pair (simulated BLE) → first-time setup → device hub
- Per device: **Status** (animated ring + demo), **Activity**, **Vents**, **Device** (contacts / unpair)
- Theme toggle (persisted)

GSM still runs on the hardware. BLE is simulated here.

```bash
cd companion
npm install
npm run dev -- --hostname 127.0.0.1 --port 43123
```

Open [http://127.0.0.1:43123](http://127.0.0.1:43123) on a phone (same Wi‑Fi) or Add to Home Screen.
