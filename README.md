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

## Repo

| Path | Role |
|------|------|
| [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) | Arduino firmware |
| [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) | 3D-printed case |
| [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md) | BOM, pin map, pitch outline |
| [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) | Visual assembly |
| [`companion/`](companion/) | Android-style companion app (pair → setup → per-device tabs) |

## Hardware

See the build guide. Upload `aeroguard_x1-1.ino` to an Arduino Uno. Serial 9600.

## Companion app

```bash
cd companion
npm install
npm run dev -- --hostname 127.0.0.1 --port 43123
```

Pair a simulated AeroGuard, finish setup (name, place, owner number), then use **Device** for contacts / unpair and **Vents** to link smart windows.
