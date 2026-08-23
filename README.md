# AeroGuard-X1

LPG leak + fire early-warning **product** for Ghanaian homes, hostels, chop bars, and cylinder areas. Competition prototype with a clear path to a market device + phone app.

## What it does

| Stage | LED | Action |
|-------|-----|--------|
| LOW | Green | Quiet early warning |
| MEDIUM | Yellow | Buzzer + SMS; app opens linked smart vents/windows |
| CRITICAL | Red | Alarm + **call + SMS** to owner |
| FIRE | Red | Alarm + **call + SMS** (fire-service SMS = Phase 2) |

**Demo button** simulates a leak on stage (no real gas). **Reset** mutes, exits demo, recalibrates.  
**No on-device servo** — ventilation is an app / smart-vent feature.

## Repo files

| File | Role |
|------|------|
| [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) | Arduino firmware |
| [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) | 3D-printed case |
| [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md) | BOM, pin map, demo pitch outline |
| [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) | Visual assembly |

## Quick start

1. Wire per the build guide (set owner phone numbers in the sketch).
2. Upload `aeroguard_x1-1.ino` to an Arduino Uno.
3. Open Serial Monitor at 9600 baud; press **Demo** to walk stages.
4. Print the case from OpenSCAD (`part = "all_export"`).

## Next

Mobile app: live status, contacts, smart vent/window control, cloud incident log.
