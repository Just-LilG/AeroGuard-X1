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

**Connectivity:** **Uno + ESP32**, or **Arduino Yún** (Wi‑Fi on the board, no ESP32). GSM (SIM800L) still needs a ~4 V buck.

## Repo

| Path | Role |
|------|------|
| [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) | Firmware (Uno or Yún) |
| [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) | ESP32 WiFi bridge (`/status`, vent command) |
| [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) | Compact 3D-printed case (126×90×36) with AeroGuard engraving |
| [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md) | Full BOM, pin map, pitch outline |
| [`AeroGuard-X1_Current_Build_Guide.md`](AeroGuard-X1_Current_Build_Guide.md) | Bench inventory (missing LM2596 / part 9) |
| [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md) | Uno + ESP32 module wires |
| [`AeroGuard-X1_Yun_Pin_Map.md`](AeroGuard-X1_Yun_Pin_Map.md) | Yún as brain (no ESP32) |
| [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) | Visual assembly |
| [`companion/`](companion/) | Android-style companion app (pair → setup → per-device tabs) |

## Hardware

See the build guide. **Uno:** upload `aeroguard_x1-1.ino` (Serial 9600) and `esp32_aeroguard_bridge.ino` to the ESP32 (115200). **Yún:** same `.ino`, board = Arduino Yún — see [`AeroGuard-X1_Yun_Pin_Map.md`](AeroGuard-X1_Yun_Pin_Map.md); do not use the ESP32 pin map.

## Companion app

```bash
cd companion
npm install
npm run dev -- --hostname 127.0.0.1 --port 43123
```

Contest UI can still simulate discovery; on hardware, the ESP32 serves live `/status` over WiFi.
