# AeroGuard-X1 — Pin Map & Wiring Reference

**This page is for the parts on the table today.**  
It matches [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino).

You **do not have** the **SD card module** (the tiny file cabinet) or the **SIM800L** (the phone chip). Leave those pins empty. The box can still show lights, screen, beeper, and Demo / Reset.

The full shopping story is in [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md). Pictures: [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html).

---

## What you have vs what to buy

Numbers match the Build Guide shopping list.

| # | Part | On the table? |
|---|------|----------------|
| 1 | Arduino Uno (the small computer / brain) | **Have** |
| 2 | MQ-5 or MQ-2 gas sensor (the nose) | **Have** |
| 3 | KY-026 flame sensor (the eye) | **Have** |
| 4 | LCD 1602 I2C screen | **Have** |
| 5 | Green / yellow / red LEDs + 220Ω resistors | **Have** |
| 6 | Buzzer (the beeper) | **Have** |
| 7 | Demo + Reset buttons | **Have** |
| 8 | SIM800L phone chip + antenna | **Buy later** |
| 9 | LM2596 buck (4V “pressure reducer” for the phone chip) | **Buy later** (needed with the SIM800L) |
| 10 | Micro SD module + card | **Buy later** |
| 11 | 5V USB power bank or battery pack | **Have** |
| 12 | Breadboard + jumper wires | **Have** |
| 13 | 10kΩ + 20kΩ resistors (SIM protect) | Keep in the bag until the phone chip arrives |
| 14 | Nano-SIM with airtime | Keep in the bag until the phone chip arrives |
| 15 | Printed case | **Have** (or print when you are ready) |
| 16 | USB **data** cable + screwdriver + multimeter | **Have** |

**Wire now:** 1–7, plus 5V / GND.  
**Do not wire:** SIM800L, SD, 4V buck, D5, D6, D10–D13.

---

## Uno pin map (today)

Think of the Uno as a farm tap board. Each hole is a tap. Empty taps stay empty.

| Uno pin | Goes to | Wire today? |
|---------|---------|-------------|
| A0 | Gas sensor analog (AO) | **Yes** |
| A1 | — | **No. Leave empty.** |
| A2 | Flame sensor analog (the module’s own A0 pin) | **Yes** |
| A3 | — | **No. Leave empty.** |
| A4 | LCD **SDA** (data) | **Yes** |
| A5 | LCD **SCL** (clock) | **Yes** |
| D2 | Green LED | **Yes** |
| D3 | Yellow LED | **Yes** |
| D4 | Red LED | **Yes** |
| D5 | Would be SIM800L TX | **No. No phone chip yet.** |
| D6 | Would be SIM800L RX | **No. No phone chip yet.** |
| D7 | Reset button → GND | **Yes** |
| D8 | Buzzer | **Yes** |
| D9 | Demo button → GND | **Yes** |
| D10 | Would be SD CS | **No. No SD module yet.** |
| D11 | Would be SD MOSI | **No** |
| D12 | Would be SD MISO | **No** |
| D13 | Would be SD SCK | **No** |
| 5V | Sensors, LCD, LEDs, buzzer, buttons | **Yes** |
| GND | Every module you plug in | **Yes. One shared drain.** |

---

## Power (today)

No 4V rail yet. Power the Uno from USB (computer or power bank).

| Wire | From | To |
|------|------|-----|
| Red | Uno **5V** | Breadboard **+** rail |
| Black | Uno **GND** | Breadboard **−** rail |

Join every module GND to that same **−** rail.

---

## 1. Gas sensor (MQ-5 or MQ-2) → A0

The nose. First minutes of heat can smell like warm electronics. That is normal.

| Module pin | Connects to |
|------------|-------------|
| VCC | 5V |
| GND | GND |
| A0 or AO (analog) | Uno **A0** |
| D0 or DO | Leave unconnected |

---

## 2. Flame sensor (KY-026) → A2

The eye. It sees flame light, not room heat.

The pin labeled **A0 on the flame board** is **not** the Uno’s A0. It goes to Uno **A2**.

| Module pin | Connects to |
|------------|-------------|
| VCC (+ / V) | 5V |
| GND (G) | GND |
| A0 analog on the module | Uno **A2** |
| D0 | Leave unconnected |

---

## 3. LCD 1602 I2C → A4 / A5

The little text screen. The backpack on the back talks **I2C** (a two-wire chat). On the Uno those two wires are always A4 and A5.

| Backpack pin | Connects to |
|--------------|-------------|
| VCC | 5V |
| GND | GND |
| SDA | Uno **A4** |
| SCL | Uno **A5** |

Blank screen? Contrast screw on the backpack. Address in the program is `0x27`. Some boards need `0x3F`.

---

## 4. LEDs → D2, D3, D4

Long leg = plus. Short leg = minus. The **220Ω** resistor is a speed bump so the lamp does not burn.

| LED | Plus → 220Ω → | Minus → |
|-----|----------------|---------|
| Green (LOW) | Uno **D2** | GND |
| Yellow (MEDIUM) | Uno **D3** | GND |
| Red (CRITICAL / FIRE) | Uno **D4** | GND |

---

## 5. Buzzer → D8

The beeper.

| Buzzer | Connects to |
|--------|-------------|
| + or S | Uno **D8** |
| − | GND |

If it has three pins: **S** to D8, **−** to GND, **+** to 5V only if the board asks for it.

---

## 6. Demo + Reset buttons → D9, D7

The program already uses an internal pull-up. Press = pin to ground. **Do not** wire buttons to 5V.

On a 4-leg button, opposite corners are a pair. Use one leg from each pair.

| Button | Leg 1 | Leg 2 |
|--------|-------|-------|
| Reset | Uno **D7** | GND |
| Demo | Uno **D9** | GND |

Label the lid **RESET** and **DEMO**.

---

## Leave empty until you buy the missing parts

### SD module (part 10) — not on the table

Do not put wires on **D10, D11, D12, D13**. The program still runs. The log file just will not appear.

When you buy it:

| SD pin | To |
|--------|-----|
| VCC | 5V if the board says 5V (some cheap boards want 3.3V — read the print) |
| GND | GND |
| CS | D10 |
| MOSI | D11 |
| MISO | D12 |
| SCK | D13 |

### SIM800L phone chip (part 8) + LM2596 (part 9) — not on the table

Do not put wires on **D5** or **D6**. Do not feed anything that looks like a phone module from the Uno **5V** pin.

The program has `GSM_ENABLED` and `SD_ENABLED` set to **false**, so Demo will not try to call or write a log file. When those parts arrive, set the matching line to `true` and re-upload.

| Later | To |
|-------|-----|
| SIM VCC | Buck **~4.0V only** — never Uno 5V |
| SIM GND | Common GND |
| SIM TX | Uno D5 (direct) |
| SIM RX | Mid-point of 10kΩ + 20kΩ from D6 |

```
Uno D6  --- 10kΩ ---+--- SIM800L RX
                    |
                  20kΩ
                    |
                   GND
```

---

## What you can test today

Upload [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino). USB **data** cable. Serial Monitor **9600**. Wait ~45 seconds of calibrating.

| Press | Lights / sound | Phone / SD |
|-------|----------------|------------|
| Demo 1 | Green. LCD LOW. Quiet. | No SMS (no phone chip) |
| Demo 2 | Yellow. Short beeps. | No SMS |
| Demo 3 | Red. Loud alarm. | No call |
| Demo 4 | Red. FIRE on screen. | No call |
| Reset | Alarm off. Calibrate again. | — |

The Vercel app is still a pretend screen for judges. It does not need these missing parts.
