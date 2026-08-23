# AeroGuard-X1

Smart gas leak detection system built around an Arduino Uno. Dual-zone gas sensing with flame fast-track, multi-stage escalation (LOW → MEDIUM → CRITICAL / EMERGENCY), GSM call/SMS alerts, SD logging, and automatic vent control.

## Repository contents

| File | Description |
|------|-------------|
| [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) | Arduino firmware (sensors, LCD, servo, SIM800L, SD log) |
| [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) | OpenSCAD enclosure (base, lid, mounts, vent) |
| [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md) | Full build guide: parts, pin map, wiring, calibration |
| [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) | Visual assembly guide (open in a browser) |

## Firmware (Arduino)

1. Install [Arduino IDE](https://www.arduino.cc/en/software) (or PictoBlox).
2. Install libraries: **LiquidCrystal I2C**, **Servo** (built-in), **SD** (built-in).
3. Open `aeroguard_x1-1.ino`, select **Arduino Uno**, then upload.
4. Follow the build guide for wiring, SIM800L power/voltage notes, and contact numbers before deploying.

## Case (OpenSCAD)

1. Install [OpenSCAD](https://openscad.org/downloads.html).
2. Open `aeroguard_x1_case.scad`.
3. Set `part = "all_export"` (default), press **F6** to render, then **File → Export → Export as STL**.
4. Slice and 3D-print.

## Docs

- Start with [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md) for parts and wiring.
- Open [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) in a browser for the visual walkthrough.
