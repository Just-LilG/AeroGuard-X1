# AeroGuard-X1 — Build Guide (v1 Demo / Pitch Kit)

**Smart LPG leak + fire early-warning device**  
Competition prototype — Arduino Uno + GSM alerts + AeroGuard contest app

This guide matches the current firmware ([`aeroguard_x1-1.ino`](aeroguard_x1-1.ino)), case ([`aeroguard_x1_case.scad`](aeroguard_x1_case.scad)), and assembly manual ([`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html)).

**Demo build = no ESP32.** Do not buy, wire, or flash an ESP32 for this pitch kit. The phone app is the contest UI on Vercel. It does **not** talk to the box. Real calls and texts come from the GSM chip (SIM800L) on the Uno.

---

## 1. What we are building

AeroGuard-X1 sits near an LPG stove or cylinder area. It learns a normal air baseline, then escalates:

| Stage | LED | Device actions | Who is notified |
|-------|-----|----------------|-----------------|
| SAFE | Off | Monitoring | — |
| LOW | **Green** | Quiet early warning on LCD | App status only (simulated in the contest UI) |
| MEDIUM | **Yellow** | Intermittent buzzer + SMS | Owner SMS; app shows linked smart vents/windows |
| CRITICAL | **Red** | Continuous alarm + **call + SMS** | Owner |
| FIRE | **Red** | Alarm + **call + SMS** | Owner (fire-service SMS = Phase 2) |

**Demo button (presentation):** each press advances LOW → MEDIUM → CRITICAL → FIRE (simulated leak). Everything else is real, including GSM calls/SMS.  
**Reset button:** mute, exit demo, recalibrate.

**Not on this device:** a motorized vent flap. Ventilation is an **app + smart vent/window** feature so the home works together.

**Multi-zone:** one hub now; extra gas sensors later. Do not buy two full units for the pitch — tell the expansion story.

---

## 2. Parts list (buy / pack)

Use these **module names** when shopping (exact PCB silkscreen may vary by seller).

This is the **demo packing list**. Buy only what is here.

| # | Role | Module / part name | Qty | Notes |
|---|------|--------------------|-----|-------|
| 1 | Controller | **Arduino Uno R3** (ATmega328P) | 1 | The small computer on the box. Official or compatible (CH340 OK). |
| 2 | Gas sensor | **MQ-2 Gas Sensor Module** *or* **MQ-5 LPG Gas Sensor Module** | 1 | Prefer **MQ-5** for LPG; breakout with AO/DO |
| 3 | Flame sensor | **IR Flame Sensor Module** (often sold as **KY-026**) | 1 | Use **analog (A0)** output |
| 4 | Display | **LCD 1602 I2C** (16×2 + **PCF8574** backpack) | 1 | I2C address usually `0x27` or `0x3F` |
| 5 | Status LEDs | **5mm LED** green, yellow, red + **220Ω** resistors | 1 each | Or **KY-011** dual LED modules if preferred |
| 6 | Alarm | **Active Buzzer Module** (e.g. **KY-012**) | 1 | Passive buzzer OK if wired to D8 |
| 7 | Buttons | **6×6mm tactile push button** (momentary) *or* **KY-004** button module | 2 | Demo + Reset |
| 8 | GSM | **SIM800L V2.0 GSM/GPRS Module** (with antenna) | 1 | Phone chip for real SMS and calls. Needs a strong 4V supply. |
| 9 | SIM power | **LM2596 DC-DC Buck Converter** (adjust to ~4.0V) | 1 | **Do not** power SIM from Uno 5V |
| 10 | Logging | **Micro SD Card Module (SPI)** (often **HW-125**) + **microSD** FAT32 | 1 | Local incident history |
| 11 | Battery | **18650 cell(s)** + **TP4056** charger *and/or* **5V USB power bank** / **MT3608** boost to 5V | 1 | Portable demo power |
| 12 | Wiring | **MB-102 breadboard** + **Dupont jumper wires** | 1 set | Or Uno proto shield |
| 13 | SIM RX protect | **Resistors 10kΩ + 20kΩ** (voltage divider) | 1 pair | Uno D6 → SIM800L RX |
| 14 | Network | **Nano-SIM** with call + SMS credit (MTN/Vodafone/AirtelTigo etc.) | 1 | Must fit SIM800L slot (adapter if needed) |
| 15 | Enclosure | **AeroGuard-X1 3D-printed case** (this repo’s SCAD → STL) | 1 | |

**Do not add to this shopping list:** an **ESP32** (a second board that can join WiFi). It is not needed for the demo. See [§10](#10-phase-2--later-optional-esp32).

**Optional (not required for v1 demo):** second **MQ-5** for multi-zone expansion later.

**Removed vs old draft:** SG90 servo, vent flap parts, second required gas sensor, orange LED, **HM-10 BLE**, **ESP32** as a required part.

**Shopping tip:** If a seller lists “MQ-2 smoke sensor,” it still detects LPG/smoke mix; for a cleaner LPG story ask for **MQ-5**.

---

## 2b. How this demo talks to people

| Path | Who | What |
|------|-----|------|
| **GSM (SIM800L)** | Always, on the box | Real SMS / call when gas or fire is high. This works even if the phone app is just a mock-up. |
| **Contest phone app** | Vercel website | Simulated discovery and status. Judges tap the UI. It does **not** reach the box. |
| **USB Serial** | Bench | Same `STATUS` / `APP_CMD` lines for debugging on a computer |

**Who does what:** the Uno keeps sensors, LEDs, buzzer, SD, and GSM. The phone app is hosted on Vercel (the contest website host). Vercel cannot see a board on your home WiFi. That is why this demo does not use WiFi.

---

## 3. Pin map

| Pin | Function |
|-----|----------|
| A0 | Gas sensor analog |
| A1 | **Leave empty for demo** (reserved for a later WiFi board) |
| A2 | Flame sensor analog |
| A3 | **Leave empty for demo** (reserved for a later WiFi board) |
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
| ~4V rail | SIM800L VCC only (shared GND with Uno) |

---

## 4. Wiring warnings (before power)

1. **SIM800L power:** separate ~3.7–4.2V capable of ~2A peaks — never Uno 5V.  
2. **SIM800L RX:** level-shift Uno 5V TX down (~2.8V logic). Use the 10k + 20k resistors.  
3. **Leave A1 and A3 empty.** Do not put a WiFi board on them for this demo.  
4. **One common GND** for Uno, SIM, battery, sensors.  
5. **SPI pins D11–D13** are taken by SD — leave free otherwise.  
6. **Flame threshold:** test your module; adjust `FLAME_DETECT_THRESHOLD` in code.  
7. **Set phone numbers** in firmware before any live GSM demo.

You do **not** set a WiFi name or password for this demo. There is no WiFi board to flash.

---

## 5. Assembly order

Do these in order. There is no ESP32 step.

1. Power rails (5V + GND; separate 4V for SIM)  
2. Gas sensor → A0  
3. Flame sensor → A2  
4. LCD I2C → A4/A5  
5. Green / Yellow / Red LEDs → D2 / D3 / D4  
6. Buzzer → D8  
7. Reset button → D7; Demo button → D9  
8. SD module → D10–D13  
9. SIM800L on 4V + D5/D6 (divider)  
10. Fit into printed case; label Demo vs Reset on the lid  
11. Upload **only** the Uno sketch [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino). Serial speed **9600**. Test the demo button.

Open [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) in a browser for the visual walkthrough.

---

## 6. Firmware behaviour (summary)

- **Calibration:** 45s average on A0 at boot and on Reset.  
- **Live mode:** % above baseline with 8s confirmation window.  
- **Demo mode:** Demo button forces stages without gas.  
- **MEDIUM:** SMS owner + `APP_CMD:VENT_OPEN` on USB Serial.  
- **CRITICAL / FIRE:** call + SMS owner.  
- **Secondary contact:** SMS only after 3 minutes if still CRITICAL/FIRE — backup check-in, **not** “enter and fix a leak.”  
- **Fire service voice call:** not used (unrealistic silent ring). Phase 2 = verified address SMS / partner.  
- **SD `gaslog.txt`:** timestamps for demos, landlords, insurance narrative — **not** claimed fire-proof. Primary history will live in the app.  
- **Phone app:** contest UI is simulated. Real alerts are GSM from the box.

---

## 7. Before you upload

Upload **one** file: [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) to the Arduino Uno.

| Setting | Change to |
|---------|-----------|
| `OWNER_CONTACT` | Real owner number (`+233…`) |
| `SECONDARY_CONTACT` | Roommate / landlord (optional) |
| `DEVICE_LABEL` | e.g. `Hostel Block A Kitchen` |
| `lcd(0x27,…)` | Match I2C scan |
| `FLAME_DETECT_THRESHOLD` | From lighter test |

Libraries: `LiquidCrystal_I2C`, `SD`, `SoftwareSerial` (built-in).

Board = **Arduino Uno**. Port = the Uno’s USB port. Serial Monitor speed = **9600**.

Do not open or upload [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) for this demo.

---

## 8. Demo script (~10 minutes) — outline

1. **Problem** — LPG in homes/hostels/chop bars; delayed response when asleep/away.  
2. **Buyer** — lead with **hostel wardens / multi-tenant housing**; chop bars & homes as expansion.  
3. **Show device** — cased unit, green/yellow/red meaning. One board: the Uno.  
4. **Live demo** — press Demo: LOW → MEDIUM (SMS) → CRITICAL (**real call**).  
5. **App story** — open the Vercel contest app. Pairing and status are simulated for judges. Smart vents/windows are shown in the app. Real calls and texts still come from the box (GSM).  
6. **Cost + market** — BOM honesty; who pays.  
7. **Q&A ready** — SD = history not fire vault; neighbor = SMS check-in not first responder; fire service = Phase 2 with address text; WiFi board is later, not this kit.

---

## 9. Case print (v2 compact)

Open [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) → `part = "all_export"` → F6 → Export STL.

| | v1 | **v2 (current)** |
|--|----|------------------|
| Outer size | 150 × 100 × 42 mm | **126 × 90 × 36 mm** |
| Look | Boxy | Rounded shell, engraved branding |
| Branding | — | **AeroGuard** on lid + front; **X1** on side; inside lid/floor text |

Prints: **base**, **lid**, **sensor mount** (print a spare for multi-zone story).  
Mounts inside: Uno standoffs, SIM / SD / 4V platforms, 4 lid screw bosses. There is also an empty shelf that can hold a WiFi board later. Leave it empty for the demo.  
No vent flap / servo parts.

---

## 10. Phase 2 / later (optional ESP32)

Skip this for the pitch kit.

The file [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) stays in the repo. An ESP32 is a second small board that can join home WiFi and show a local `/status` page. That only works on the same WiFi. The Vercel contest app still cannot reach it.

If you add it later: pins A1 and A3, plus a 10k + 20k divider into the ESP32 RX pin. It is not required to buy, wire, or flash that board for this demo.
