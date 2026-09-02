# AeroGuard-X1 — Build Guide (v1 Product Spec)

**Smart LPG leak + fire early-warning device**  
Competition / market prototype — Arduino Uno + **ESP32 WiFi** + AeroGuard app (vents via app)

This guide matches the current firmware ([`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) on the Uno, [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) on the ESP32), case ([`aeroguard_x1_case.scad`](aeroguard_x1_case.scad)), and assembly manual ([`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html)).

**The ESP32 is required.** It is the WiFi radio that lets a phone read live status and send “open vents” from a browser. Do not skip it, and do not leave it unwired.

Jump to: **[§7 ESP32 WiFi bridge (required)](#7-esp32-wifi-bridge-required)** — what it is, wiring, flashing, phone test, fallback WiFi, troubleshooting.

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

**Two boards, two jobs (plain language):**

- The **Arduino Uno** is the farm: sensors, lights, buzzer, SIM card calls/texts, SD card log.
- The **ESP32** is the water channel to the phone: it takes the Uno’s status line and puts it on WiFi so a browser (and later the app) can read it. It can also send `APP_CMD:VENT_OPEN` back to the Uno when you tap “open vents.”

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
| 11 | **App link (required)** | **ESP32 DevKit** (WiFi) | 1 | DevKit / “ESP32 Dev Module”; USB for power + upload |
| 12 | Battery | **18650 cell(s)** + **TP4056** charger *and/or* **5V USB power bank** / **MT3608** boost to 5V | 1 | Portable demo power |
| 13 | Wiring | **MB-102 breadboard** + Dupont jumper wires | 1 set | Or Uno proto shield |
| 14 | SIM RX protect | **Resistors 10kΩ + 20kΩ** (voltage divider) | 1 pair | Uno D6 → SIM800L RX |
| 15 | **ESP32 RX protect (required)** | **Resistors 10kΩ + 20kΩ** (voltage divider) | 1 pair | Uno A3 (5V) → ESP32 GPIO16 (3.3V). Do not skip. |
| 16 | Network | **Nano-SIM** with call + SMS credit (MTN/Vodafome/AirtelTigo etc.) | 1 | Must fit SIM800L slot (adapter if needed) |
| 17 | Enclosure | **AeroGuard-X1 3D-printed case** (this repo’s SCAD → STL) | 1 | Includes an ESP32 platform (~51 × 28 mm) |
| 18 | USB cable | **Data** USB cable (not charge-only) | 1 | Needed to flash the ESP32 |

**Optional (not required for v1 demo):** second **MQ-5** for multi-zone expansion later.

**Removed vs old draft:** SG90 servo, vent flap parts, second required gas sensor, orange LED, **HM-10 BLE**. The ESP32 covers local + remote WiFi. Bluetooth is not part of this build.

**Shopping tip:** If a seller lists “MQ-2 smoke sensor,” it still detects LPG/smoke mix; for a cleaner LPG story ask for **MQ-5**. Buy a common **ESP32 DevKit** (30-pin or 38-pin is fine as long as you can find GPIO16 and GPIO17).

---

## 3. How remote access works

| Path | Who | What |
|------|-----|------|
| **GSM (SIM800L)** | Always | SMS / call when you are away — emergency alerts |
| **ESP32 WiFi (required)** | Home network, or the device’s own fallback hotspot | Live status + vent commands |
| **USB Serial** | Bench | Same `STATUS` / `APP_CMD` lines for debugging |

**Who does what:** the Uno keeps sensors, LEDs, buzzer, SD, and GSM. The ESP32 only copies those talk-lines onto WiFi (`esp32_aeroguard_bridge.ino`).

**Hardware test is a phone browser**, not the companion app yet. The contest companion can still *pretend* it found a device. On real hardware you open `http://<ip>/status` in Chrome or Safari.

---

## 4. Pin map

### Arduino Uno

| Pin | Function |
|-----|----------|
| A0 | Gas sensor analog |
| A1 | **ESP32 GPIO17 (TX) → Uno RX** (app talk-line) |
| A2 | Flame sensor analog |
| A3 | **Uno TX → ESP32 GPIO16 (RX)** via 10k+20k divider (5V down to 3.3V) |
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

### ESP32 DevKit (required)

| ESP32 pin | Goes to | Notes |
|-----------|---------|-------|
| **GPIO16** (RX2) | Junction of the 10k/20k divider from **Uno A3** | 3.3V max — never wire Uno A3 straight here |
| **GPIO17** (TX2) | **Uno A1** (straight wire) | 3.3V is safe for the Uno |
| **GND** | **Uno GND** | Required. One common ground. |
| USB / 5V VIN | USB power bank or PC | USB is fine for the demo |

The printed case has an ESP32 shelf about **51 × 28 mm**. Antenna / USB should face the case opening.

---

## 5. Wiring warnings (before power)

1. **SIM800L power:** separate ~3.7–4.2V capable of ~2A peaks — never Uno 5V.
2. **SIM800L RX:** knock Uno 5V TX down (~2.8–3.3V) with a 10k+20k divider.
3. **ESP32 is 3.3V on its signal pins.** Uno A3 is **5V**. Wire A3 through a **10k + 20k divider** into ESP32 **GPIO16**. A straight 5V wire can kill the ESP32.
4. **GPIO17 → Uno A1 does not need a divider.** The ESP32’s 3.3V “high” is already enough for the Uno.
5. **One common GND** for Uno, ESP32, SIM, battery, sensors. If grounds are not tied, the talk-line will not work.
6. **SPI pins D11–D13** are taken by SD — leave free otherwise.
7. **Flame threshold:** test your module; adjust `FLAME_DETECT_THRESHOLD` in code.
8. **Set phone numbers** in the Uno firmware before any live GSM demo.
9. **Set WiFi name and password** in [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) (`WIFI_SSID` / `WIFI_PASS`) before you upload. If you leave the placeholders, the ESP32 still works: it opens its own hotspot (see [§7.6](#76-fallback-hotspot-when-home-wifi-is-not-set)).

---

## 6. Assembly order

Do these in order. The ESP32 is a real step — not an optional extra.

1. Power rails (5V + GND; separate 4V for SIM)
2. Gas sensor → A0
3. Flame sensor → A2
4. LCD I2C → A4/A5
5. Green / Yellow / Red LEDs → D2 / D3 / D4
6. Buzzer → D8
7. Reset button → D7; Demo button → D9
8. SD module → D10–D13
9. SIM800L on 4V + D5/D6 (divider)
10. **ESP32 wiring** — GPIO16 / GPIO17 / GND + the 10k+20k divider (full how-to in [§7.2](#72-exact-wiring))
11. **Flash both boards** — Uno sketch (Serial **9600**) and ESP32 bridge (Serial **115200**). ESP32 upload steps: [§7.3–7.5](#73-arduino-ide-add-the-esp32-board-list)
12. Fit into printed case; label Demo vs Reset on the lid
13. **Phone test** — Serial Monitor IP, then browser `/status` and `/vent/open` ([§7.5](#75-upload-and-first-test))

Open [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) in a browser for the visual walkthrough.

---

## 7. ESP32 WiFi bridge (required)

This chapter is the full ESP32 path: what it is, how to wire it, how to put the program on it, and how to prove it from your phone.

### 7.1 What the ESP32 is

Think of two jobs:

| Board | Everyday picture | What it actually does here |
|-------|------------------|----------------------------|
| **Arduino Uno** | The **farm** | Watches gas and flame, drives LEDs / buzzer / LCD, sends SMS and calls, writes the SD log. Speaks a simple text line: `STATUS level=... demo=... gas=... vent=APP` |
| **ESP32** | The **water channel** to the phone | Sits on WiFi. Copies that STATUS line into a web page the phone can open. If you hit “open vents,” it sends `APP_CMD:VENT_OPEN` back to the Uno. |

The Uno cannot join home WiFi by itself. The ESP32 can. That is why both boards are required.

You will load the file [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) onto the ESP32. The Uno keeps [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino).

### 7.2 Exact wiring

Use **three signal paths** plus power. Do this on the breadboard before you stuff the case.

| From | To | Parts on that wire |
|------|----|--------------------|
| Uno **A3** (TX, 5V) | ESP32 **GPIO16** (RX2) | **Must** go through the 10k + 20k divider below |
| ESP32 **GPIO17** (TX2) | Uno **A1** (RX) | Straight jumper — no divider |
| ESP32 **GND** | Uno **GND** | Straight jumper — required |
| ESP32 USB (or 5V VIN) | Phone charger / PC / power bank | Fine for demo. Do not feed ESP32 5V into a 3.3V-only pin. |

**ASCII diagram (copy this onto paper while you wire):**

```
  Arduino Uno                              ESP32 DevKit
  -----------                              ------------
                                           3.3V logic — be gentle

  A3 (TX, 5V) ----[ 10kΩ ]----+---- GPIO16 (RX2)
                              |
                           [ 20kΩ ]
                              |
                             GND ---------------- GND
                              |
  A1 (RX)  <------------------+---- GPIO17 (TX2)   (straight wire)
                              |
  GND      -------------------´

  USB 5V to Uno                            USB 5V to ESP32
```

**Voltage divider how-to (the 5V → 3.3V knock-down):**

The Uno’s A3 pin shouts at **5 volts**. The ESP32’s GPIO16 ear can only take about **3.3 volts**. Two resistors in a chain quietly drop the shout.

1. Put the **10kΩ** resistor from **Uno A3** to an empty breadboard row. Call that row the **junction**.
2. Put the **20kΩ** resistor from that **same junction** to **GND**.
3. Run one jumper from the **junction** to **ESP32 GPIO16**.

Math if you want it: `5V × 20k / (10k + 20k) ≈ 3.3V`. That is the safe level.

Which resistor goes where:

- **10k from A3, 20k to GND** → about 3.3V at GPIO16. **Do this.**
- **20k from A3, 10k to GND** → about 1.7V. The ESP32 may hear nothing.

**Do not** connect Uno A3 straight to GPIO16.

On most DevKit boards, GPIO16 / GPIO17 are labelled **RX2 / TX2** or just **16 / 17**. They are *not* the USB serial pins (those are usually GPIO1 / GPIO3).

### 7.3 Arduino IDE: add the ESP32 board list

You need the free **Arduino IDE** (2.x is fine). The Uno is already in the IDE. The ESP32 is not, until you add it.

1. Open Arduino IDE.
2. **File → Preferences** (on macOS: **Arduino IDE → Settings**).
3. Find **Additional boards manager URLs**.
4. Paste this address (add a comma if another URL is already there):

   `https://espressif.github.io/arduino-esp32/package_esp32_index.json`

5. Click OK.
6. Open **Tools → Board → Boards Manager**.
7. Search **esp32**.
8. Install **esp32** by **Espressif Systems**. Wait until it finishes (it is a large download).
9. Close Boards Manager.
10. **Tools → Board → esp32 → ESP32 Dev Module**.

   That exact name — **ESP32 Dev Module** — is the one this guide uses. Generic “ESP32 DevKit” boards almost always work with it.

11. **Tools → Port** — pick the new USB port that appears when the ESP32 is plugged in. Unplug/replug if you are not sure which one it is.

You now have two “personalities” in the IDE: Uno when you upload `aeroguard_x1-1.ino`, and **ESP32 Dev Module** when you upload `esp32_aeroguard_bridge.ino`. Switch the board and the port each time.

### 7.4 Set your WiFi name and password

1. Open [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) in the IDE.
2. Near the top, find:

```c
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";
```

3. Replace the two quoted strings with your **home (or hotspot) WiFi name and password**. Keep the quotes.

Example (fake values):

```c
const char* WIFI_SSID = "JadonHostelWiFi";
const char* WIFI_PASS = "secret123";
```

Use 2.4 GHz WiFi. Many ESP32 boards cannot join a 5 GHz-only network.

If you are at a contest venue and do not know the WiFi yet, leave the placeholders. The board will open its own hotspot instead ([§7.6](#76-fallback-hotspot-when-home-wifi-is-not-set)). You can re-upload later with the real name.

### 7.5 Upload and first test

**Cable:** use a **data** USB cable. A charge-only cable will power the board but will **not** show a Port. If Tools → Port is empty, swap the cable first.

**Upload:**

1. Board = **ESP32 Dev Module**, Port = the ESP32’s USB port.
2. Click **Upload**.
3. If it sits on `Connecting........` and then fails:
   - Hold the **BOOT** button (sometimes labelled **IO0**) on the ESP32.
   - Click Upload again (or keep holding BOOT as the “Connecting” dots start).
   - Release BOOT when you see the compile/upload progress move. Some boards also want a tap of **EN / RESET** just before that.
4. Wait for **“Leaving... Hard resetting via RTS pin...”** or a plain “Done uploading.”

**Read the IP address:**

1. **Tools → Serial Monitor**.
2. Set the speed (bottom of the window) to **115200**. (The Uno uses 9600 — this one is faster.)
3. Press the ESP32 **EN / RESET** button once.
4. You should see either:
   - `Remote URL: http://192.168.x.x` — it joined your WiFi. Write that address down.
   - `AP mode SSID=AeroGuard-X1  http://192.168.4.1` — it could not join, so it opened its own hotspot. Go to [§7.6](#76-fallback-hotspot-when-home-wifi-is-not-set).

**Phone / laptop browser test (this is the hardware test):**

Join the **same** WiFi as the ESP32 (your home WiFi, or the AeroGuard hotspot). Then open:

| What you type | What “good” looks like |
|---------------|------------------------|
| `http://<the-ip>/status` | JSON text with `"level"`, `"demo"`, `"gas"`, `"raw"`, `"remote":true`. The `raw` field looks like `STATUS level=SAFE demo=0 gas=0 vent=APP` once the Uno is talking. |
| `http://<the-ip>/vent/open` | JSON like `{"ok":true,"cmd":"APP_CMD:VENT_OPEN"}`. The ESP32 also sends that command to the Uno. |
| `http://<the-ip>/` | A tiny AeroGuard page with links to `/status` and open-vents. |

Use **http** not https. There is no login page.

The companion app still **simulates discovery** for the contest UI. Do not wait for the app to “find” the box. If `/status` loads, the hardware WiFi path works.

### 7.6 Fallback hotspot when home WiFi is not set

If `WIFI_SSID` is empty or still `YOUR_WIFI_SSID`, **or** the named WiFi cannot be joined in time, the ESP32 makes its own WiFi network:

| | Value |
|---|--------|
| Network name (SSID) | `AeroGuard-X1` |
| Password | `aeroguard` |
| Address on that network | usually `http://192.168.4.1` |

On your phone:

1. Open WiFi settings.
2. Join **AeroGuard-X1** / password **aeroguard**.
3. Browser: `http://192.168.4.1/status` and `http://192.168.4.1/vent/open`.

Change `AP_PASS` in the sketch before a public demo if you do not want the default password.

### 7.7 Troubleshooting

| What you see | Try this |
|--------------|----------|
| No Port in the IDE | Data USB cable (not charge-only). Try another USB socket. Windows may need a **CP2102** or **CH340** driver — match the USB chip printed on the ESP32. |
| Upload stuck on `Connecting...` | Hold **BOOT**, click Upload, release when it starts writing. Try **Tools → Upload Speed → 115200**. Unplug other serial devices. |
| Serial Monitor garbage | Speed must be **115200** for the ESP32 (9600 is the Uno). Close and reopen Monitor, tap EN/RESET. |
| `AP mode` when you expected home WiFi | SSID/password typo, or 5 GHz-only network. Move closer. 2.4 GHz guest network often works. |
| Phone cannot open `/status` | Phone is on a different WiFi. Typed `https://` instead of `http://`. Wrong IP (check Serial Monitor again). Some guest networks block device-to-device; use the fallback hotspot to prove the board. |
| `/status` loads but `level` stays empty / SAFE forever | Uno not uploaded, or **A3/A1 swapped**, or **missing GND**, or **missing divider** (ESP32 may have crashed). Confirm Uno Serial 9600 shows `STATUS level=...` lines. |
| `/vent/open` returns ok but Uno ignores it | GPIO17 not on A1, or Uno sketch old. Uno Serial should print `ESP32 cmd: APP_CMD:VENT_OPEN`. |
| ESP32 resets in a loop when Uno is wired | Classic 5V-on-RX damage path. Power off. Check the 10k/20k divider on A3 → GPIO16. |
| Board gets hot | Unplug immediately. Look for 5V on a 3.3V pin or a shorted 5V/GND jumper. |

---

## 8. Firmware behaviour (summary)

- **Calibration:** 45s average on A0 at boot and on Reset.
- **Live mode:** % above baseline with 8s confirmation window.
- **Demo mode:** Demo button forces stages without gas.
- **MEDIUM:** SMS owner + `APP_CMD:VENT_OPEN` on USB Serial **and** the ESP32 talk-line.
- **CRITICAL / FIRE:** call + SMS owner.
- **Secondary contact:** SMS only after 3 minutes if still CRITICAL/FIRE — backup check-in, **not** “enter and fix a leak.”
- **Fire service voice call:** not used (unrealistic silent ring). Phase 2 = verified address SMS / partner.
- **SD `gaslog.txt`:** timestamps for demos, landlords, insurance narrative — **not** claimed fire-proof. Primary history will live in the app.
- **ESP32:** must be fitted and flashed. It publishes `/status` JSON over WiFi and can forward vent-open to the Uno.

---

## 9. Before you upload

Do **both** boards. Switching board + port in the IDE each time is easy to forget.

### Arduino Uno — `aeroguard_x1-1.ino`

| Setting | Change to |
|---------|-----------|
| Board / port | **Arduino Uno** (or “Arduino AVR Boards → Uno”) and the Uno’s USB port |
| Serial Monitor | **9600** |
| `OWNER_CONTACT` | Real owner number (`+233…`) |
| `SECONDARY_CONTACT` | Roommate / landlord (optional) |
| `DEVICE_LABEL` | e.g. `Hostel Block A Kitchen` |
| `lcd(0x27,…)` | Match I2C scan |
| `FLAME_DETECT_THRESHOLD` | From lighter test |

Libraries: `LiquidCrystal_I2C`, `SD`, `SoftwareSerial` (built-in).

### ESP32 — `esp32_aeroguard_bridge.ino` (required)

| Setting | Change to |
|---------|-----------|
| Board / port | **ESP32 Dev Module** and the ESP32’s USB port |
| Serial Monitor | **115200** |
| `WIFI_SSID` | Your 2.4 GHz WiFi name (or leave placeholder to use the hotspot) |
| `WIFI_PASS` | That network’s password |
| `AP_SSID` / `AP_PASS` | Defaults `AeroGuard-X1` / `aeroguard` — change for a public demo if you want |

---

## 10. Demo script (~10 minutes) — outline

1. **Problem** — LPG in homes/hostels/chop bars; delayed response when asleep/away.
2. **Buyer** — lead with **hostel wardens / multi-tenant housing**; chop bars & homes as expansion.
3. **Show device** — cased unit, green/yellow/red meaning. Point at both boards: Uno = farm, ESP32 = channel to the phone.
4. **Live demo** — press Demo: LOW → MEDIUM (SMS) → CRITICAL (**real call**).
5. **App / phone story** — on the same WiFi, open `http://<esp32-ip>/status` in the phone browser (hardware proof). Then show the companion UI. Smart vents/windows open together. GSM still calls when you are away. If venue WiFi is hostile, join the **AeroGuard-X1** hotspot and use `http://192.168.4.1/status`.
6. **Cost + market** — BOM honesty; who pays.
7. **Q&A ready** — SD = history not fire vault; neighbor = SMS check-in not first responder; fire service = Phase 2 with address text; no Bluetooth module — WiFi is the ESP32.

---

## 11. Case print (v2 compact)

Open [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) → `part = "all_export"` → F6 → Export STL.

| | v1 | **v2 (current)** |
|--|----|------------------|
| Outer size | 150 × 100 × 42 mm | **126 × 90 × 36 mm** |
| Look | Boxy | Rounded shell, engraved branding |
| Branding | — | **AeroGuard** on lid + front; **X1** on side; inside lid/floor text |

Prints: **base**, **lid**, **sensor mount** (print a spare for multi-zone story).  
Mounts inside: Uno standoffs, **ESP32 platform (about 51 × 28 mm)**, SIM / SD / 4V platforms, 4 lid screw bosses.  
No vent flap / servo parts.

Seat the ESP32 so USB and the antenna side stay reachable.

---

## 12. Next (after this hardware pack)

The ESP32 is already part of the build — flashed, wired, and proven with `/status` ([§7](#7-esp32-wifi-bridge-required)). What is still later:

1. Point the AeroGuard companion at the ESP32 `/status` URL (contest UI can stay simulated until that hook is live).
2. Optional Phase 2: cloud tunnel / MQTT so the phone works off the home WiFi.
3. Optional: change the fallback hotspot password before a public booth.
