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

**Connectivity:** Arduino Uno runs sensors + **GSM**. An **ESP32** bridges status/commands over **WiFi** so the phone can reach the unit remotely on the home network (not Bluetooth-only).

## Repo

| Path | Role |
|------|------|
| [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) | Arduino Uno firmware |
| [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) | ESP32 WiFi bridge (`/status`, vent command) |
| [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) | Compact 3D-printed case (126×90×36) with AeroGuard engraving |
| [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md) | BOM, pin map, pitch outline |
| [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) | Visual assembly |
| [`companion/`](companion/) | Android-style companion app (pair → setup → per-device tabs) |

## Hardware

See the build guide. Upload `aeroguard_x1-1.ino` to an Arduino Uno (Serial 9600) and `esp32_aeroguard_bridge.ino` to the ESP32 (Serial 115200). Set WiFi SSID/password in the ESP32 sketch for remote access.

## Companion app

```bash
cd companion
npm install
npm run dev -- --hostname 127.0.0.1 --port 43123
```

Contest UI can still simulate discovery; on hardware, the ESP32 serves live `/status` over WiFi.
