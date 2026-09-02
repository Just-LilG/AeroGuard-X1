# AeroGuard companion

Phone-first web app for the contest pitch: **Mi Home–style device home** with **light/dark liquid-glass** UI.

## How this fits the box

| Layer | Role |
|-------|------|
| **This contest UI** | Simulated discovery and status. The Vercel site does **not** talk to the hardware. |
| **GSM on the box** | Real SMS and calls from the **SIM800L** (phone chip) on the **Arduino Uno**. |

Judges tap the app. Real alerts still come from the SIM in the case.

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
