# AeroGuard-X1 — Pin Map & Module Wiring Reference

Module-by-module wiring for the **Uno + ESP32** pack. If the **Yún** is the brain, use [`AeroGuard-X1_Yun_Pin_Map.md`](AeroGuard-X1_Yun_Pin_Map.md) instead — pins are different.

Matches [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) and [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino).

---

## Arduino Uno reference points

- **5V** and **GND** — main rail for every module in this stage
- Digital **D2–D13**
- Analog **A0–A5**

---

## Full pin map

| Pin | Function | Wire now? |
|-----|----------|-----------|
| A0 | Gas sensor analog | Yes |
| A1 | ESP32 TX2 → Uno RX (app UART) | Yes |
| A2 | Flame sensor analog | Yes |
| A3 | Uno TX → ESP32 RX2 (app UART; **via divider**) | Yes |
| A4 | LCD SDA | Yes |
| A5 | LCD SCL | Yes |
| D2 | Green LED (LOW) | Yes |
| D3 | Yellow LED (MEDIUM) | Yes |
| D4 | Red LED (CRITICAL / FIRE) | Yes |
| D5 | SIM800L TX → Uno RX | **Later** (needs 4 V rail) |
| D6 | Uno TX → SIM800L RX, via divider | **Later** |
| D7 | Reset button → GND | Yes |
| D8 | Buzzer | Yes |
| D9 | Demo button → GND | Yes |
| D10 | SD CS | Yes |
| D11 | SD MOSI (fixed SPI) | Yes |
| D12 | SD MISO (fixed SPI) | Yes |
| D13 | SD SCK (fixed SPI) | Yes |

---

## 1. Gas sensor (Flying Fish MQ module) → A0

| Module pin | Connects to |
|---|---|
| VCC | Uno 5V |
| GND | Uno GND |
| DO | Not used |
| AO | Uno **A0** |

---

## 2. Flame sensor → A2

| Module pin | Connects to |
|---|---|
| VCC (+ / V) | Uno 5V |
| GND (G) | Uno GND |
| D0 | Not used |
| A0 *(the module’s own analog pin)* | Uno **A2** |

The flame board’s pin labeled **A0** is **not** the Uno’s A0. It goes to Uno **A2**.

---

## 3. LCD 1602 + I2C backpack → A4 / A5

| Backpack pin | Connects to |
|---|---|
| GND | Uno GND |
| VCC | Uno 5V |
| SDA | Uno **A4** |
| SCL | Uno **A5** |

If the screen stays blank, I2C-scan — some backpacks are `0x3F` instead of firmware `0x27`.

---

## 4. LEDs → D2, D3, D4

Longer leg = anode (+), shorter = cathode (−).

| LED | Anode (+) → 220Ω → | Cathode (−) → |
|---|---|---|
| Green | Uno **D2** | Uno GND |
| Yellow | Uno **D3** | Uno GND |
| Red | Uno **D4** | Uno GND |

The 220Ω can sit anywhere on the anode wire.

---

## 5. Buzzer → D8

| Buzzer | Connects to |
|---|---|
| Red (+) / + | Uno **D8** |
| Black (−) / − | Uno GND |

Active module or piezo disc — firmware uses `tone()` on D8.

---

## 6. Demo + Reset buttons → D9, D7

4-leg tactile: opposite corners are paired — one leg from each pair.

| Button | Leg 1 | Leg 2 |
|---|---|---|
| Demo | Uno **D9** | Uno GND |
| Reset | Uno **D7** | Uno GND |

No extra resistor — firmware uses `INPUT_PULLUP`.

---

## 7. SD card module → D10–D13

| Module pin | Connects to |
|---|---|
| GND | Uno GND |
| 3V3 | Not used |
| 5V | Uno 5V |
| CS | Uno **D10** |
| MOSI | Uno **D11** |
| SCK | Uno **D13** |
| MISO | Uno **D12** |

---

## 8. ESP32 DevKit ↔ Uno → A1 / A3

**Power:** ESP32 on its **own USB** for now. Do not power it from the Uno.

| Connection | Path |
|---|---|
| Uno **A3** (TX, 5 V) → ESP32 **GPIO16 (RX2)** | **Divider:** Uno A3 → 10kΩ → junction → ESP32 RX2; junction → 20kΩ → GND |
| ESP32 **GPIO17 (TX2)** → Uno **A1** (RX) | Direct — 3.3 V is safe into the Uno |
| ESP32 GND → Uno GND | Required |

Only the Uno→ESP32 direction needs the divider (5 V into a 3.3 V-max pin).

---

## Not yet wired (waiting on part 9 — LM2596)

You **have** the SIM800L, antenna, Nano-SIM, and the second 10k/20k pair. Leave them off the rail until the buck is set to ~4.0 V.

- SIM800L VCC / GND (buck 4 V only — never Uno 5 V or the 18650 directly)
- SIM800L TX → Uno **D5**
- Uno **D6** → SIM800L RX via 10kΩ / 20kΩ

Full steps: [`AeroGuard-X1_Current_Build_Guide.md`](AeroGuard-X1_Current_Build_Guide.md) §6.
