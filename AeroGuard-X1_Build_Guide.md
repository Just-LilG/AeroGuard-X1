# AeroGuard-X1 — Build Guide (v1 Product Spec)

**Smart LPG leak + fire early-warning device**  
Competition / market prototype — Arduino Uno + AeroGuard app (vents via app)

This guide matches the current firmware ([`aeroguard_x1-1.ino`](aeroguard_x1-1.ino)), case ([`aeroguard_x1_case.scad`](aeroguard_x1_case.scad)), and assembly manual ([`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html)).

---

## 1. What we are building

AeroGuard-X1 sits near an LPG stove or cylinder area. It learns a normal air baseline, then escalates:

| Stage | LED | Device actions | Who is notified |
|-------|-----|----------------|-----------------|
| SAFE | Off | Monitoring | — |
| LOW | **Green** | Quiet early warning on LCD | App status only |
| MEDIUM | **Yellow** | Intermittent buzzer + SMS | Owner SMS; app opens linked smart vents/windows |
| CRITICAL | **Red** | Continuous alarm + **call + SMS** | Owner |
| FIRE | **Red** | Alarm + **call + SMS** | Owner (fire-service SMS = Phase 2) |

**Demo button (presentation):** each press advances LOW → MEDIUM → CRITICAL → FIRE (simulated leak). Everything else is real, including GSM calls/SMS.  
**Reset button:** mute, exit demo, recalibrate.

**Not on this device:** a motorized vent flap. Ventilation is an **app + smart vent/window** feature so the home works together.

**Multi-zone:** one hub now; extra gas sensors later. Do not buy two full units for the pitch — tell the expansion story.

---

## 2. Parts list (buy / pack)

Use these **module names** when shopping (exact PCB silkscreen may vary by seller).

| # | Role | Module / part name | Qty | Notes |
|---|------|--------------------|-----|-------|
| 1 | Controller | **Arduino Uno R3** (ATmega328P) | 1 | Official or compatible (CH340 OK) |
| 2 | Gas sensor | **MQ-2 Gas Sensor Module** *or* **MQ-5 LPG Gas Sensor Module** | 1 | Prefer **MQ-5** for LPG; breakout with AO/DO |
| 3 | Flame sensor | **IR Flame Sensor Module** (often sold as **KY-026**) | 1 | Use **analog (A0)** output |
| 4 | Display | **LCD 1602 I2C** (16×2 + **PCF8574** backpack) | 1 | I2C address usually `0x27` or `0x3F` |
| 5 | Status LEDs | **5mm LED** green, yellow, red + **220Ω** resistors | 1 each | Or **KY-011** dual LED modules if preferred |
| 6 | Alarm | **Active Buzzer Module** (e.g. **KY-012**) | 1 | Passive buzzer OK if wired to D8 |
| 7 | Buttons | **6×6mm tactile push button** (momentary) *or* **KY-004** button module | 2 | Demo + Reset |
| 8 | GSM | **SIM800L V2.0 GSM/GPRS Module** (with antenna) | 1 | Needs strong 4V supply |
| 9 | SIM power | **LM2596 DC-DC Buck Converter** (adjust to ~4.0V) | 1 | **Do not** power SIM from Uno 5V |
| 10 | Logging | **Micro SD Card Module (SPI)** (often **HW-125**) + **microSD** FAT32 | 1 | Local incident history |
| 11 | App link (remote) | **ESP32 DevKit** (WiFi) | 1 | Bridges Uno ↔ phone over WiFi; replaces HM-10 |
| 12 | Battery | **18650 cell(s)** + **TP4056** charger *and/or* **5V USB power bank** / **MT3608** boost to 5V | 1 | Portable demo power |
| 13 | Wiring | **MB-102 breadboard** + ** Dupont jumper wires** | 1 set | Or Uno proto shield |
| 14 | SIM RX protect | **Resistors 10kΩ + 20kΩ** (voltage divider) | 1 pair | Uno D6 → SIM800L RX |
| 15 | ESP32 RX protect | **Resistors ~10kΩ + 20kΩ** (optional divider) | 1 pair | Uno A3 (5V TX) → ESP32 RX if no shifter |
| 16 | Network | **Nano-SIM** with call + SMS credit (MTN/Vodafone/AirtelTigo etc.) | 1 | Must fit SIM800L slot (adapter if needed) |
| 17 | Enclosure | **AeroGuard-X1 3D-printed case** (this repo’s SCAD → STL) | 1 | |

**Optional (not required for v1 demo):** second **MQ-5** for multi-zone expansion later.

**Removed vs old draft:** SG90 servo, vent flap parts, second required gas sensor, orange LED, **HM-10 BLE** (ESP32 covers local + remote WiFi).

**Shopping tip:** If a seller lists “MQ-2 smoke sensor,” it still detects LPG/smoke mix; for a cleaner LPG story ask for **MQ-5**. Use an **ESP32 DevKit** for the app link so users can open status off-site on the home network (cloud tunnel later).

---

## 2b. How remote access works

| Path | Who | What |
|------|-----|------|
| **GSM (SIM800L)** | Always | SMS / call when you are away — emergency alerts |
| **ESP32 WiFi** | Home network / remote | Live status + vent commands in the AeroGuard app |
| **USB Serial** | Bench | Same `STATUS` / `APP_CMD` lines for debugging |

**Architecture:** Uno keeps sensors, LEDs, buzzer, SD, and GSM. ESP32 only bridges those lines to WiFi (`esp32_aeroguard_bridge.ino`). Contest companion can still simulate the link until WiFi credentials are set.

---

## 3. Pin map

| Pin | Function |
|-----|----------|
| A0 | Gas sensor analog |
| A1 | **ESP32 TX → Uno RX** (app UART) |
| A2 | Flame sensor analog |
| A3 | **Uno TX → ESP32 RX** (app UART; level-shift to 3.3V) |
| A4 | LCD SDA |
| A5 | LCD SCL |
| D2 | Green LED (LOW) |
| D3 | Yellow LED (MEDIUM) |
| D4 | Red LED (CRITICAL / FIRE) |
| D5 | SIM800L TX → Uno RX |
| D6 | Uno TX → SIM800L RX **via voltage divider** |
| D7 | Reset / mute / recalibrate button → GND |
| D8 | Buzzer |
| D9 | **Demo** button → GND |
| D10 | SD CS |
| D11 | SD MOSI (fixed SPI) |
| D12 | SD MISO (fixed SPI) |
| D13 | SD SCK (fixed SPI) |
| 5V / GND | Sensors, LCD, LEDs, buzzer, buttons, SD |
| 3.3V / 5V | ESP32 per board (USB OK for demo) |
| ~4V rail | SIM800L VCC only (shared GND with Uno) |

---

## 4. Wiring warnings (before power)

1. **SIM800L power:** separate ~3.7–4.2V capable of ~2A peaks — never Uno 5V.  
2. **SIM800L RX:** level-shift Uno 5V TX down (~2.8V logic).  
3. **ESP32 RX:** Uno A3 is 5V — use a divider / level shifter into ESP32 RX (3.3V max).  
4. **One common GND** for Uno, ESP32, SIM, battery, sensors.  
5. **SPI pins D11–D13** are taken by SD — leave free otherwise.  
6. **Flame threshold:** test your module; adjust `FLAME_DETECT_THRESHOLD` in code.  
7. **Set phone numbers** in firmware before any live GSM demo.  
8. **Set WiFi SSID/password** in [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) for remote access.

---

## 5. Assembly order

1. Power rails (5V + GND; separate 4V for SIM)  
2. Gas sensor → A0  
3. Flame sensor → A2  
4. LCD I2C → A4/A5  
5. Green / Yellow / Red LEDs → D2 / D3 / D4  
6. Buzzer → D8  
7. Reset button → D7; Demo button → D9  
8. SD module → D10–D13  
9. SIM800L on 4V + D5/D6 (divider)  
10. ESP32 on A1/A3 (WiFi bridge — see [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino); can leave unwired for GSM-only rehearsal)  
11. Fit into printed case; label Demo vs Reset on the lid  
12. Upload Uno sketch + ESP32 bridge; Serial 9600 (Uno) / 115200 (ESP32); test demo button

Open [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) in a browser for the visual walkthrough.

---

## 6. Firmware behaviour (summary)

- **Calibration:** 45s average on A0 at boot and on Reset.  
- **Live mode:** % above baseline with 8s confirmation window.  
- **Demo mode:** Demo button forces stages without gas.  
- **MEDIUM:** SMS owner + `APP_CMD:VENT_OPEN` on USB Serial **and** ESP32 UART.  
- **CRITICAL / FIRE:** call + SMS owner.  
- **Secondary contact:** SMS only after 3 minutes if still CRITICAL/FIRE — backup check-in, **not** “enter and fix a leak.”  
- **Fire service voice call:** not used (unrealistic silent ring). Phase 2 = verified address SMS / partner.  
- **SD `gaslog.txt`:** timestamps for demos, landlords, insurance narrative — **not** claimed fire-proof. Primary history will live in the app.  
- **ESP32:** publishes `/status` JSON over WiFi; can forward vent open to the Uno.

---

## 7. Before you upload

| Setting | Change to |
|---------|-----------|
| `OWNER_CONTACT` | Real owner number (`+233…`) |
| `SECONDARY_CONTACT` | Roommate / landlord (optional) |
| `DEVICE_LABEL` | e.g. `Hostel Block A Kitchen` |
| `lcd(0x27,…)` | Match I2C scan |
| `FLAME_DETECT_THRESHOLD` | From lighter test |

Libraries: `LiquidCrystal_I2C`, `SD`, `SoftwareSerial` (built-in).

---

## 8. Demo script (~10 minutes) — outline

1. **Problem** — LPG in homes/hostels/chop bars; delayed response when asleep/away.  
2. **Buyer** — lead with **hostel wardens / multi-tenant housing**; chop bars & homes as expansion.  
3. **Show device** — cased unit, green/yellow/red meaning.  
4. **Live demo** — press Demo: LOW → MEDIUM (SMS) → CRITICAL (**real call**).  
5. **App story** — phone shows status over **ESP32 WiFi** (remote on the home network); smart vents/windows open together. GSM still calls when you are away.  
6. **Cost + market** — BOM honesty; who pays.  
7. **Q&A ready** — SD = history not fire vault; neighbor = SMS check-in not first responder; fire service = Phase 2 with address text; BLE not required when ESP32 is fitted.

---

## 9. Case print

Open [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) → `part = "all_export"` → F6 → Export STL.  
Prints: base, lid, sensor mount(s). No vent flap / servo parts.

---

## 10. Next (after this hardware pack)

1. Flash [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) and join WiFi.  
2. Point the AeroGuard companion at the ESP32 `/status` endpoint (contest UI can stay simulated until that hook is live).  
3. Optional Phase 2: cloud tunnel / MQTT so the phone works off the home LAN.
