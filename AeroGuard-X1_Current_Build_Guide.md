# AeroGuard-X1 — Current Build Guide

Bench build against the parts you actually have. Product spec remains [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md). Module-by-module wires: [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md).

**Inventory date:** 29 Aug 2026  

**Brain (choose one):** Arduino **Uno + ESP32** (original) **or** Arduino **Yún** (on-board Wi‑Fi, no ESP32). You have a Yún — pin map: [`AeroGuard-X1_Yun_Pin_Map.md`](AeroGuard-X1_Yun_Pin_Map.md). Do not mix the two maps on one board.

---

## 1. What is on the bench

BOM numbers match the Build Guide §2.

| # | Part | Status | Use now |
|---|------|--------|---------|
| 1 | Arduino Uno R3 | **Have** | Brain **or** spare if you use the Yún |
| 1b | Arduino Yún | **Have** | Alternate brain — see Yun pin map; drop ESP32 |
| 2 | MQ-2 / MQ-5 gas module | **Have** | A0 |
| 3 | IR flame module | **Have** | A2 |
| 4 | LCD 1602 I2C | **Have** | A4 / A5 |
| 5 | Green / yellow / red LEDs + 220Ω | **Have** | D2 / D3 / D4 |
| 6 | Buzzer | **Have** | D8 |
| 7 | Two tactile buttons | **Have** | Demo D9, Reset D7 |
| 8 | SIM800L + antenna | **Have** | **Do not power yet** (needs #9) |
| 9 | LM2596 buck (~4.0 V for SIM) | **Missing** | Blocks GSM VCC |
| 10 | Micro SD SPI module + card | **Have** | D10–D13 |
| 11 | ESP32 DevKit | **Have** | **Uno path only** — skip if the Yún is the brain |
| 12 | 18650 / TP4056 / USB bank | **Have** | 5 V for Uno (not SIM) |
| 13 | Breadboard + Dupont wires | **Have** | |
| 14 | 10kΩ + 20kΩ (SIM RX divider) | **Have** | Keep aside until §6 |
| 15 | 10kΩ + 20kΩ (ESP32 RX divider) | **Have** | Uno A3 → ESP32 RX2 |
| 16 | Nano-SIM with credit | **Have** | Insert only when SIM is on 4 V |
| 17 | 3D-printed case | **Have** | After the bench works |

**Optional later:** second MQ-5 (multi-zone story). Not required for this pack.

---

## 2. What you can finish without the buck

You can assemble a **full contest bench** except live SMS/call:

- Gas + flame → LCD + LEDs + buzzer  
- Demo / Reset buttons  
- SD `gaslog.txt`  
- ESP32 WiFi `/status` and vent command into the Uno  
- Companion app stories (demo leak, vents)

GSM stays **unpowered**. Firmware still compiles and runs; `sendSMS` / `callNumber` will do nothing useful until §6.

**Never** feed SIM800L from Uno 5V, ESP32 5V, or the 18650 pack raw. Peak current can collapse the rail and brown-out the Uno.

---

## 3. Power for this stage

| Rail | Source | Feeds |
|------|--------|--------|
| 5 V | Uno USB **or** power bank / boost into Uno | Sensors, LCD, LEDs, buzzer, buttons, SD |
| 3.3 V / 5 V (ESP32) | **ESP32 USB cable** | ESP32 only — do not power ESP32 from the Uno |
| ~4.0 V | **Wait for LM2596** | SIM800L VCC only |
| GND | One common ground | Uno + ESP32 + (later) SIM + battery pack |

---

## 4. Wire now (order)

Follow the pin map for each module. Tick as you go.

1. Breadboard 5 V + GND from the Uno.  
2. Gas sensor → A0.  
3. Flame sensor analog → A2 (module pin labeled A0 is **not** Uno A0).  
4. LCD I2C → A4 SDA / A5 SCL.  
5. LEDs + 220Ω → D2 / D3 / D4.  
6. Buzzer → D8.  
7. Reset → D7; Demo → D9 (to GND, internal pull-ups).  
8. SD → D10 CS, D11 MOSI, D12 MISO, D13 SCK.  
9. ESP32: USB power; GPIO17 → Uno A1; Uno A3 → **10k/20k divider** → ESP32 GPIO16; common GND.  
10. Leave SIM800L VCC, D5, and D6 disconnected. Keep the second 10k/20k pair for SIM RX.

Then flash [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino):

- **Uno:** Board = Arduino Uno, Serial 9600, ESP32 sketch at 115200.  
- **Yún:** Board = Arduino Yún, Serial 9600 USB; **do not** flash the ESP32 bridge. Wait for Linux if you want WLAN. Smoke test is still LCD + Demo/Reset.

**Smoke test:** LCD shows Safe after ~45 s calibration. Demo button: Low → Medium → Critical → Fire. Reset recalibrates. Serial `STATUS` lines; Uno+ESP32 also `http://<esp32-ip>/status`.

---

## 5. Demo without GSM

| Stage | On the box (now) | GSM (after §6) |
|-------|------------------|----------------|
| LOW | Green LED, LCD | — |
| MEDIUM | Yellow + beep + `APP_CMD:VENT_OPEN` | Owner SMS |
| CRITICAL / FIRE | Red + alarm | Owner call + SMS |

Pitch line until the buck arrives: *“GSM is on the box; we are not powering the modem until the 4 V rail is regulated.”* App vents still work over ESP32.

---

## 6. When the LM2596 arrives (part 9)

Do this **after** the rest of the bench is stable.

1. Power the buck from the pack / 5–12 V input (not from Uno 5V if you can avoid it).  
2. **Adjust output to ~4.0 V with a meter before connecting the SIM800L.**  
3. SIM800L VCC → buck 4 V out; SIM800L GND → common GND.  
4. SIM800L TX → Uno **D5** (Yún: **D11**).  
5. Uno **D6** (Yún: **D12**) → 10kΩ → junction → SIM800L RX; junction → 20kΩ → GND.  
6. Fit the Nano-SIM (credit for call + SMS). Antenna on.  
7. Set `OWNER_CONTACT` / `SECONDARY_CONTACT` / `DEVICE_LABEL` in the Uno sketch.  
8. CRITICAL on Demo will **really call** — warn whoever holds that phone.

If the modem resets during a call, the 4 V rail is sagging — thicker wires, closer cap, or a supply that can do ~2 A peaks.

---

## 7. Related files

| File | Role |
|------|------|
| [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md) | Uno + ESP32 wires |
| [`AeroGuard-X1_Yun_Pin_Map.md`](AeroGuard-X1_Yun_Pin_Map.md) | Yún as brain (no ESP32) |
| [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md) | Full product BOM + pitch |
| [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) | Visual steps (SIM still listed last) |
| [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) | Uno firmware |
| [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) | WiFi bridge |
