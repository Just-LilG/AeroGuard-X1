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

| # | Item | Qty | Notes |
|---|------|-----|-------|
| 1 | Arduino Uno | 1 | |
| 2 | MQ-2 or MQ-5 gas sensor | 1 | Kitchen / cylinder area |
| 3 | Flame sensor (analog IR) | 1 | |
| 4 | 16×2 LCD + I2C backpack | 1 | Address usually `0x27` or `0x3F` |
| 5 | LED green, yellow, red | 1 each | + 220Ω ×3 |
| 6 | Buzzer | 1 | |
| 7 | Push buttons | 2 | Demo + Reset |
| 8 | SIM800L GSM module | 1 | |
| 9 | Buck converter (~4V for SIM800L) | 1 | **Do not** power SIM from Uno 5V |
| 10 | MicroSD module + FAT32 card | 1 | Local incident history |
| 11 | HM-10 (or HC-05) BLE module | 1 | Phone app link (reserved pins) |
| 12 | Li-ion pack + 5V regulator | 1 | Backup / portable demo power |
| 13 | Breadboard + jumpers | 1 set | Or proto shield inside case |
| 14 | Voltage divider resistors for SIM RX | 1 pair | e.g. 10k + 20k |
| 15 | Active SIM with call + SMS credit | 1 | |
| 16 | 3D-printed case (this repo’s SCAD) | 1 | |

**Removed vs old draft:** servo motor, vent flap mechanism, second required gas sensor, orange LED.

---

## 3. Pin map

| Pin | Function |
|-----|----------|
| A0 | Gas sensor analog |
| A1 | **Reserved** — HM-10 BLE RX (app) |
| A2 | Flame sensor analog |
| A3 | **Reserved** — HM-10 BLE TX (app) |
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
| 5V / GND | Sensors, LCD, LEDs, buzzer, buttons, SD, BLE VCC as specified |
| ~4V rail | SIM800L VCC only (shared GND with Uno) |

---

## 4. Wiring warnings (before power)

1. **SIM800L power:** separate ~3.7–4.2V capable of ~2A peaks — never Uno 5V.  
2. **SIM800L RX:** level-shift Uno 5V TX down (~2.8V logic).  
3. **One common GND** for Uno, SIM, battery, sensors.  
4. **SPI pins D11–D13** are taken by SD — leave free otherwise.  
5. **Flame threshold:** test your module; adjust `FLAME_DETECT_THRESHOLD` in code.  
6. **Set phone numbers** in firmware before any live GSM demo.

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
10. HM-10 on A1/A3 (app phase — can leave unwired for GSM-only rehearsal)  
11. Fit into printed case; label Demo vs Reset on the lid  
12. Upload [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino), Serial 9600, test demo button

Open [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) in a browser for the visual walkthrough.

---

## 6. Firmware behaviour (summary)

- **Calibration:** 45s average on A0 at boot and on Reset.  
- **Live mode:** % above baseline with 8s confirmation window.  
- **Demo mode:** Demo button forces stages without gas.  
- **MEDIUM:** SMS owner + `APP_CMD:VENT_OPEN` on Serial (app listens later).  
- **CRITICAL / FIRE:** call + SMS owner.  
- **Secondary contact:** SMS only after 3 minutes if still CRITICAL/FIRE — backup check-in, **not** “enter and fix a leak.”  
- **Fire service voice call:** not used (unrealistic silent ring). Phase 2 = verified address SMS / partner.  
- **SD `gaslog.txt`:** timestamps for demos, landlords, insurance narrative — **not** claimed fire-proof. Primary history will live in the app.

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
5. **App story** — phone shows status; smart vents/windows open together (mock OK if BLE not finished).  
6. **Cost + market** — BOM honesty; who pays.  
7. **Q&A ready** — SD = history not fire vault; neighbor = SMS check-in not first responder; fire service = Phase 2 with address text.

---

## 9. Case print

Open [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) → `part = "all_export"` → F6 → Export STL.  
Prints: base, lid, sensor mount(s). No vent flap / servo parts.

---

## 10. Next (after this hardware pack)

Plan and build the **AeroGuard mobile app**: live status, contacts, smart vent/window links, incident log sync.
