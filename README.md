# AeroGuard-X1

LPG leak + fire early-warning **product** for Ghanaian homes, hostels, chop bars, and cylinder areas.

| Stage | LED | Action |
|-------|-----|--------|
| LOW | Green | Quiet early warning |
| MEDIUM | Yellow | Buzzer + SMS; app opens linked smart vents/windows |
| CRITICAL | Red | Alarm + **call + SMS** to owner |
| FIRE | Red | Alarm + **call + SMS** (fire-service SMS = Phase 2) |

**Demo button** on the box (and in the app) simulates a leak. **Reset** mutes, exits demo, recalibrates.  
**No on-device servo** — ventilation is an app / smart-vent feature.

**Connectivity (this demo):** the **Arduino Uno** runs sensors, lights, buzzer, SD log, and **GSM** (the SIM800L chip that sends real SMS and makes real calls). The phone app is the **Vercel contest UI**. Pairing and status in the app are simulated. The app does **not** talk to the box.

An **ESP32** (a second board for WiFi) is **later / optional**. Do not buy, wire, or flash it for the pitch kit. The sketch stays in the repo for Phase 2.

## Repo

| Path | Role |
|------|------|
| [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) | Arduino Uno firmware (the demo brain) |
| [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) | Optional later WiFi sketch — **not** part of the demo packing list |
| [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) | Compact 3D-printed case (126×90×36) with AeroGuard engraving |
| [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md) | Demo BOM, pin map, pitch outline |
| [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) | Visual assembly |
| [`companion/`](companion/) | Android-style companion app (pair → setup → per-device tabs) |

## Hardware

See the build guide. For the demo, upload **only** `aeroguard_x1-1.ino` to an Arduino Uno (Serial 9600). Set owner phone numbers in that sketch. Do not set WiFi. Do not flash the ESP32 file.

## Companion app

```bash
cd companion
npm install
npm run dev -- --hostname 127.0.0.1 --port 43123
```

Contest UI simulates discovery and status. Real alerts are GSM call/SMS from the box.
