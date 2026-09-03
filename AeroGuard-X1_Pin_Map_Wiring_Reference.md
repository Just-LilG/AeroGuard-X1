# AeroGuard-X1 — Pin Map & Wiring Reference

**This page is for the parts on the table today.**  
It matches [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino).

You **have** the **SIM800L** (the phone chip that texts and calls).  
You **do not have** the **SD card module** (the tiny file cabinet). Leave **D10–D13** empty. The box still lights, beeps, and can send real SMS/calls.

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
| 8 | SIM800L phone chip + antenna | **Have** |
| 9 | LM2596 buck (4V “pressure reducer” for the phone chip) | **Have** — must be set to ~4.0V before SIM VCC |
| 10 | Micro SD module + card | **Buy later — this is the only missing part** |
| 11 | 5V USB power bank or battery pack | **Have** |
| 12 | Breadboard + jumper wires | **Have** |
| 13 | 10kΩ + 20kΩ resistors (SIM protect) | **No 20k.** Use two 10k in a line, or 10k+10k |
| 14 | Nano-SIM with airtime | **Have** |
| 15 | Printed case | **Have** (or print when you are ready) |
| 16 | USB **data** cable + screwdriver + multimeter | **Have** |

**Wire now:** 1–9 and 11–16.  
**Do not wire:** SD module. Leave **D10, D11, D12, D13** empty.

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
| D5 | SIM800L **TX** (chip talks → Uno listens) | **Yes** |
| D6 | SIM800L **RX** through the 10k + 20k divider | **Yes** |
| D7 | Reset button → GND | **Yes** |
| D8 | Buzzer | **Yes** |
| D9 | Demo button → GND | **Yes** |
| D10 | Would be SD CS | **No. No SD module.** |
| D11 | Would be SD MOSI | **No** |
| D12 | Would be SD MISO | **No** |
| D13 | Would be SD SCK | **No** |
| 5V | Sensors, LCD, LEDs, buzzer, buttons | **Yes** — **not** SIM VCC |
| GND | Every module, plus the 4V buck | **Yes. One shared drain.** |
| ~4V from buck | **SIM800L VCC only** | **Yes** after you measure ~4.0V |

---

## Power (today)

Two taps. One drain.

| Wire | From | To |
|------|------|-----|
| Red | Uno **5V** | Breadboard **+** rail (small parts only) |
| Black | Uno **GND** | Breadboard **−** rail |
| Buck input | 5V USB bank (or battery pack) | LM2596 **IN** |
| Buck output | LM2596 **OUT** after you measure **~4.0V** | SIM800L **VCC only** |
| Buck GND | LM2596 GND | Same **−** rail |

**Never** feed the SIM800L from the Uno **5V** pin. That pin is a thin hose. The chip gulps current when it calls and will reset or die.

Measure the buck with a multimeter **before** the SIM VCC wire goes on.

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

## 7. SIM800L phone chip → D5 / D6 + 4V

The mini phone. Antenna on. Nano-SIM in the slot (gold pads down, cut corner matching the drawing).

**Set 4.0V first:**

1. Leave SIM **VCC** disconnected.
2. Power the LM2596 input.
3. Meter the output. Turn the screw until you see about **4.0V**.
4. Then connect SIM **VCC** to that output.

| SIM800L | Connects to |
|---------|-------------|
| VCC | Buck **4V out** (never Uno 5V) |
| GND | Common GND |
| TX | Uno **D5** (direct) |
| RX | Mid-point of the divider from **D6** |
| Antenna | Screwed on |

**Divider.** You did **not** get a 20kΩ. Do **not** run a bare D6 wire into SIM RX.

**Option A — three 10kΩ (same as 10k + 20k):** two 10k in a line to ground (that pair is 20k), one 10k from D6 to the middle.

```
Uno D6  --- 10kΩ --------+--- SIM800L RX
                         |
                    10kΩ + 10kΩ
                         |
                        GND
```

**Option B — only two 10kΩ:** one from D6 to SIM RX, one from SIM RX to GND (~2.5V). Still a safe drop.

```
Uno D6  --- 10kΩ ---+--- SIM800L RX
                    |
                  10kΩ
                    |
                   GND
```

A 22kΩ from a shop can stand in for 20k. The SIM RX wire always comes off the **middle**.

SIM TX to D5 is direct. The chip speaks at ~3.3V. The Uno can hear that.

In the program, `GSM_ENABLED` is **true**. Put real numbers in `OWNER_CONTACT` before you press Demo to MEDIUM or CRITICAL. Those stages send a **real text** and a **real call**.

Many boards blink fast while hunting, then **slow blink** when they have joined the network.

---

## Leave empty — SD module (part 10)

You do **not** have this part. Do not put wires on **D10, D11, D12, D13**. Keep `SD_ENABLED` as **false**. The program still runs. Serial will say `No SD module (OK for this bench).`

When you buy it:

| SD pin | To |
|--------|-----|
| VCC | 5V if the board says 5V (some cheap boards want 3.3V — read the print) |
| GND | GND |
| CS | D10 |
| MOSI | D11 |
| MISO | D12 |
| SCK | D13 |

Then set `SD_ENABLED` to **true** and upload again. The file is `gaslog.txt` — a demo log, not a fire-proof vault.

---

## What you can test today

Upload [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino). USB **data** cable. Serial Monitor **9600**. Wait about **5 seconds** of sniffing air (or press Demo to skip). Warn the person who holds the owner phone.

| Press | Lights / sound | Phone / SD |
|-------|----------------|------------|
| Demo 1 | Green. LCD LOW. Quiet. | No SMS |
| Demo 2 | Yellow. Short beeps. | **Real SMS** to owner |
| Demo 3 | Red. Loud alarm. | **Real call**, then SMS |
| Demo 4 | Red. FIRE on screen. | **Real call**, then SMS |
| Reset | Alarm off. Calibrate again. | — |

No `gaslog.txt` until the SD board arrives. The Vercel app is still a pretend screen for judges.
