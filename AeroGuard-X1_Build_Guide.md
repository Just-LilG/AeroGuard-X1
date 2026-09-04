# AeroGuard-X1 — Build Guide

**The product you assemble:** a small box that watches for LPG gas leaks and fire, then **calls and texts** the owner.

This is the full demo kit. One brain board. One phone chip. Sensors, lights, screen, alarm, buttons, and a case.

Start here. The visual walkthrough is [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) (open it in a browser). The pin-for-pin wiring for **today’s parts** is [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md). The program you upload is [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino).

---

## 1. What we are building

Think of a kitchen with an LPG cylinder. Gas should stay in the pipe, like water in a hose. If it leaks, you want a warning **early**, not after someone is already in trouble.

AeroGuard-X1 sits near the stove or cylinder. It sniffs the air. It also watches for flame. When things get worse, it gets louder and it uses a real phone chip to reach people.

| Stage | Light | What the box does | Who is reached |
|-------|--------|-------------------|----------------|
| SAFE | Off | Quiet. Learning / watching. | Nobody |
| LOW | **Green** | Quiet early warning on the screen | Nobody by phone yet |
| MEDIUM | **Yellow** | Beep-beep + **SMS** (a text message) | Owner text |
| CRITICAL | **Red** | Loud alarm + **call + SMS** | Owner phone rings |
| FIRE | **Red** | Loud alarm + **call + SMS** | Owner (fire-service text comes later, not this kit) |

**Demo button:** press it to pretend a leak is getting worse. LOW → MEDIUM → CRITICAL → FIRE. Lights, screen, buzzer, and phone alerts still run for real. You do not need to open a gas valve.

**Reset button:** stop the alarm. Leave the pretend-leak. The box sniffs a new “normal air” baseline, like tasting clean water before you judge a muddy stream.

**Not on this box:** a motor that opens a window. The contest phone app **shows** smart vents. The box itself does not move a flap.

**One gas zone for the pitch.** Tell judges you can add more noses later. Do not buy two full kits for the demo.

---

## 2. How the box and the phone app fit together

There are **two** things in the demo. They do different jobs.

| Piece | What it is | What it does |
|-------|------------|--------------|
| **The box** | Hardware you wire and case | Sniffs gas, sees flame, lights LEDs, beeps, writes a log, **sends real SMS and makes real calls** |
| **The contest app** | A website on **Vercel** (a host for the pitch UI) | Looks like a phone app. Pairing and live status are **pretend**. Judges tap it. It does **not** talk to the box. |

**Real alerts = GSM.** GSM is the same kind of mobile network your phone uses. The chip is a **SIM800L**. Put a real SIM card in it (the tiny card from MTN, Telecel, AirtelTigo, and so on). Give it airtime. When the box hits MEDIUM it texts. When it hits CRITICAL or FIRE it **calls**, then texts.

The Vercel app cannot reach a box on a table in Accra. That is fine. You show the app for the product story. You show the box for the real call.

USB on a computer is only for loading the program and reading debug lines. Serial speed is **9600** (that is just “how fast the chat on the cable runs”).

---

## 3. Shopping list

Buy only what is in this table. Search shops by the **module name**.

### On the table today

You **have** the **SIM800L** phone chip (#8) and a **battery** (#11). You **do not have** the **LM2596** 4V board (#9) or the **SD** module (#10).

**You do not need the LM2596.** Feed SIM **VCC** from your **3.7V cell** — see [§3.1](#31-get-calls-working-this-week-no-lm2596). Never a resistor. Never Uno 5V. If the battery is only a USB power bank (a brick with a USB hole), that 5V is for the Uno, not for SIM VCC. The cell we mean is the fat tube (18650) or a flat phone pack.

**SIM800L-only pin map:** [`AeroGuard-X1_SIM800L_Pin_Map.md`](AeroGuard-X1_SIM800L_Pin_Map.md) · picture: [`AeroGuard-X1_SIM800L_Pin_Map.html`](AeroGuard-X1_SIM800L_Pin_Map.html). You **do not have a 20kΩ** — use 10k stand-ins for the D6 divider. Leave **D10–D13** empty.

Full pin tables: [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md).

| # | Role | What to buy | Qty | On the table? | Plain words |
|---|------|-------------|-----|---------------|-------------|
| 1 | Brain | **Arduino Uno R3** (ATmega328P). Clone with CH340 chip is OK. | 1 | **Have** | The small computer. All other parts plug into it. |
| 2 | Gas nose | **MQ-5 LPG Gas Sensor Module** (or **MQ-2** if MQ-5 is hard to find). Breakout with **AO** and **DO** pins. | 1 | **Have** | Sniffs LPG / smoke mix. Prefer MQ-5 for cooking gas. |
| 3 | Flame eye | **IR Flame Sensor Module** (often **KY-026**) | 1 | **Have** | Sees a flame’s infrared light. Use the **analog (A0)** pin on the module. |
| 4 | Screen | **LCD 1602 I2C** — 16 characters × 2 lines with a **PCF8574** backpack | 1 | **Have** | The little green/blue text screen. I2C means only two data wires. |
| 5 | Lights | **5mm LED**: 1 green, 1 yellow, 1 red + three **220Ω** resistors | 1 set | **Have** | Stage lights. Resistors are tiny “speed bumps” so the LEDs do not burn. |
| 6 | Alarm | **Active buzzer module** (often **KY-012**) | 1 | **Have** | The beeper. |
| 7 | Buttons | Two **6×6mm tactile push buttons**, or two **KY-004** button modules | 2 | **Have** | **Demo** and **Reset**. Momentary = click and it springs back. |
| 8 | Phone chip | **SIM800L V2.0 GSM/GPRS module** with antenna | 1 | **Have** | Sends SMS and makes calls. Needs strong ~4V power. |
| 9 | SIM power | **LM2596 DC-DC buck converter** | 1 | **Buy later — not required for the pitch if you use a 3.7V cell** | Nice to have. A 18650 or old phone battery already makes ~3.7V. |
| 10 | Log | **Micro SD card module (SPI)** (often **HW-125**) + a **microSD** card formatted **FAT32** | 1 | **Buy later** | Writes a simple event file. Not a fire-proof vault. |
| 11 | Battery | **18650** cell (and/or 5V USB power bank) | 1 | **Have** | **3.7V tube/phone cell → SIM VCC.** 5V USB bank → Uno only. |
| 12 | Wiring | **MB-102 breadboard** + **Dupont jumper wires** (male-male, male-female) | 1 set | **Have** | Breadboard = plastic hole grid so you can plug wires without soldering. |
| 13 | SIM protect | **10kΩ** resistor + **20kΩ** resistor | 1 pair | **No 20k** — see workaround below | Voltage divider. Drops the Uno’s 5V talk-line so the SIM chip is not hurt. |
| 14 | Network | **Nano-SIM** with call + SMS credit | 1 | **Have** | Must fit the SIM800L slot. Use an adapter if your SIM is larger. |
| 15 | Case | Printed **AeroGuard-X1** case — ready-to-print [STL files](aeroguard_x1_case_base.stl) (source: [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad)) | 1 | **Have** | Base, lid, sensor clip, ×2 button caps. Lid takes two 6×6mm switches. See [§10](#10-case-print). |
| 16 | Tools | USB **data** cable (not charge-only), small screwdriver, **multimeter** | 1 | **Have** | Multimeter = the meter that reads volts. You need it to set 4.0V later. |

**Optional, not required:** extra MQ-5 for a “second kitchen” story later.

**Do not buy for this kit:** a servo motor, a vent flap, a Bluetooth dongle, or a second brain board.

**Shop tip:** if a listing says “MQ-2 smoke sensor,” it still sees LPG mixed with smoke. For a cleaner cooking-gas story, ask for **MQ-5**.

---

## 3.1 Get calls working this week (no LM2596)

You **have** a battery. The program already has texts and calls turned **on** (`PHONE_ALERTS` is **true**). The missing LM2596 only makes 4V from 5V. Your rechargeable cell is **already** about 3.7V — that is the right pressure.

Think of the LM2596 as a tap reducer. You do not need it. The tank is already at the right height.

**Use the 3.7V cell**, not a USB hole:

- **Fat tube (18650):** bump end is plus. If it sits in a charger board, take wires from the **battery holder**, not from any 5V USB / boost hole.
- **Flat phone pack:** pads marked **+** and **−**. Ignore extra pads.

A USB power bank (a brick with a USB hole) is **5V**. That feeds the **Uno**, not SIM VCC.

### Wire the cell (unplug USB first)

```
Cell PLUS (+)  --------  SIM800L VCC
Cell MINUS (−) --------  SIM800L GND  and  Uno GND
                         (one shared drain)
```

- Tape the metal sides so plus and minus cannot touch.
- Skip a puffy, leaking, or hot cell.
- Plug USB back in for the Uno. The cell feeds **only** the SIM.

Then upload [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino). Warn the person who holds `+233557164067`. Network LED: fast blink = hunting, **slow blink** = on the network. Demo → MEDIUM = real SMS. Demo → CRITICAL = real call.

Full talk-wire steps stay in [§6.9](#69-sim800l--calls-from-a-37v-cell).

---

## 4. Pin map (Arduino Uno)

The Uno has rows of holes. **Digital** pins (D2, D3, …) are mostly on/off. **Analog** pins (A0, A1, …) can read a number from a sensor.

Pin-for-pin module tables: [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md).

| Uno pin | Goes to | Wire today? |
|---------|---------|-------------|
| A0 | Gas sensor analog out | **Yes** |
| A1 | — | **Leave empty** |
| A2 | Flame sensor analog out | **Yes** |
| A3 | — | **Leave empty** |
| A4 | LCD **SDA** (data) | **Yes** |
| A5 | LCD **SCL** (clock) | **Yes** |
| D2 | Green LED | **Yes** |
| D3 | Yellow LED | **Yes** |
| D4 | Red LED | **Yes** |
| D5 | SIM800L **TX** (chip talks → Uno listens) | **Yes** |
| D6 | SIM800L **RX** through the two-resistor divider | **Yes** |
| D7 | Reset button, other leg to GND | **Yes** |
| D8 | Buzzer | **Yes** |
| D9 | Demo button, other leg to GND | **Yes** |
| D10 | SD **CS** | **No** — no SD module yet |
| D11 | SD **MOSI** | **No** |
| D12 | SD **MISO** | **No** |
| D13 | SD **SCK** | **No** |
| 5V | Sensors, LCD, LEDs, buzzer, buttons | **Yes** — not SIM VCC |
| GND | **Every** module (including SIM GND) | **Yes** |
| ~4V | **SIM800L VCC only** | **Yes** from a **3.7V cell** (18650 or old phone battery). Never Uno 5V. |

A1 and A3 stay empty. D10–D13 stay empty until the SD board arrives.

---

## 5. Power warnings (read before any power)

1. **The SIM800L is thirsty.** When it calls, it gulps current (around **2A** peaks). It needs about **4.0 volts** (3.7–4.2V is the safe window) from a **separate** source — not a pin on the Uno. The usual part is an **LM2596** (you do **not** have this yet). **Never** take SIM power from the Uno **5V** pin or the Uno **3.3V** pin. Those pins are thin hoses. The chip will reset or die.
2. **Feed SIM VCC from your 3.7V cell.** You have the battery. See [§3.1](#31-get-calls-working-this-week-no-lm2596). **Never** Uno 5V or 3.3V. **Never** a USB power-bank 5V hole on VCC.
3. **A resistor cannot replace the LM2596 or the cell.** The 10k resistors are only for the D6 *talk* wire, like a quiet-down on a garden hose. Power is a different job. When the phone chip calls, it gulps a big gulp, then sips. A resistor’s drop changes with every gulp, so the voltage would jump around, collapse, or cook the resistor.
4. **Common ground.** Uno GND, battery GND, SIM GND (when you wire it), and every sensor GND must join. Same drain for every pipe. If you later add an LM2596, its GND joins this same rail.
5. **Voltage divider on D6.** The Uno speaks at 5V. The SIM800L listen pin wants about 2.8V. **Do not** run a bare wire from D6 to SIM RX.

   **You do not have a 20kΩ.** Use this instead:

   - **Best:** two **10kΩ** resistors in a line (leg to leg). That pair *is* 20k. Use it as the resistor down to GND. One more **10kΩ** goes from D6 to the middle (SIM RX). You need **three 10k** total for this.
   - **If you only have two 10kΩ:** one from D6 to SIM RX, one from SIM RX to GND. That gives about 2.5V. A bit lower than 3.3V, but still a safe drop. Many SIM800L boards accept it.
   - **If you only have one 10kΩ:** buy a **20kΩ** or a second **10kΩ** before you wire D6. Do not skip the drop.

   A 22kΩ is close enough to 20k if a shop has that instead.
6. **SIM TX to D5 is direct.** The chip speaks at ~3.3V. The Uno can hear that. No divider on that wire.
7. **One USB data cable** for the computer. Charge-only cables look the same and will not upload the program.
8. **Set owner phone numbers in the program** before you demo a real call. See [§7](#7-set-phone-numbers-and-upload-the-program).

---

## 6. Step-by-step assembly

Do these in order. Unplug USB and batteries while you move wires. Lights and screen run from USB. Skip the LM2596 and skip the SD module. For calls, add a **3.7V cell** to SIM VCC ([§3.1](#31-get-calls-working-this-week-no-lm2596)).

Open the HTML guide beside this page if you want the same steps with bigger cards. Or use the pin sheet: [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md).

### 6.1 Power rails

**Goal:** a 5V strip for the small parts, and one shared GND. SIM VCC comes from a **3.7V cell**, not from the Uno.

1. Put the Uno on the table. Note **5V**, **GND**, and the numbered pins.
2. Run a red jumper from Uno **5V** to the breadboard **+** rail.
3. Run a black jumper from Uno **GND** to the breadboard **−** rail.
4. Skip the LM2596. You do not need it if a 3.7V cell feeds SIM VCC.
5. **Do not** put SIM VCC on the Uno 5V pin or the 3.3V pin.

### 6.2 Gas sensor (MQ-5 or MQ-2)

**What it is:** a heated nose. First power-on it can smell “warm electronics” for a few minutes. That is normal.

Module pins (names printed on the board):

| Module pin | To |
|------------|----|
| VCC | 5V rail |
| GND | GND rail |
| A0 or AO (analog) | Uno **A0** |
| D0 or DO (digital) | **Leave unconnected** |

Clip the sensor in the printed **MQ** mount later so air from the grill can reach it.

### 6.3 Flame sensor (KY-026)

**What it is:** an eye for flame light, not a heat thermometer.

| Module pin | To |
|------------|----|
| VCC | 5V |
| GND | GND |
| A0 (analog) | Uno **A2** |
| D0 | Leave unconnected |

Aim the black LED-looking sensor toward where a flame would be. After the program is loaded you can flick a lighter **far from any gas** and watch the number in Serial. If FIRE never trips, raise or lower `FLAME_DETECT_THRESHOLD` in the program (see §7).

### 6.4 LCD screen (1602 I2C)

**What it is:** a 16×2 character screen. The **backpack** is the small board glued/soldered on the back. That backpack talks **I2C** (a two-wire chat). On the Uno, I2C is always **A4** and **A5**.

| LCD pin | To |
|---------|----|
| VCC | 5V |
| GND | GND |
| SDA | Uno **A4** |
| SCL | Uno **A5** |

If the screen stays blank after upload: contrast screw on the backpack, backlight jumper, and I2C address. Address is usually `0x27` or `0x3F`. The program starts at `0x27`. Change that one number if needed.

The program is set for a **16×2** screen (`LiquidCrystal_I2C lcd(0x27, 16, 2);`). A bigger 20×4 with the same 4 wires still works — extra rows stay blank. To fill all four rows, change `16, 2` to `20, 4` and upload again.

### 6.5 Status LEDs

**What they are:** three stage lamps. Green = LOW. Yellow = MEDIUM. Red = CRITICAL or FIRE.

For a bare 5mm LED:

1. Long leg = **anode** (plus). Short leg = **cathode** (minus).
2. Plus → **220Ω** resistor → Uno pin. Minus → GND.
3. Green to **D2**. Yellow to **D3**. Red to **D4**.

If you bought a 3-pin LED module, use **S** (signal) to the pin, **−** to GND, and **+** to 5V only if the module needs it. Follow the printing on that board.

### 6.6 Buzzer

**What it is:** the beeper. An **active** buzzer makes its own tone when the pin goes high.

| Buzzer | To |
|--------|----|
| + or S | Uno **D8** |
| − | GND |

If it has three pins (S, +, −), put **S** on D8, **−** on GND, and **+** on 5V if the module asks for it.

MEDIUM = short beeps. CRITICAL / FIRE = continuous.

### 6.7 Buttons (Demo and Reset)

The program already turns on an **internal pull-up**. That means the pin sits HIGH until you press. Press connects the pin to GND (LOW).

| Button | One leg | Other leg |
|--------|---------|-----------|
| Reset | Uno **D7** | GND |
| Demo | Uno **D9** | GND |

Do not wire buttons to 5V. The lid is already labelled **RESET** and **DEMO** so you do not mix them in front of judges.

Each **6×6mm tactile switch** presses up into the square pocket under the lid (a dab of glue holds it), and a printed **button cap** drops into the round hole on top. Solder short wires from the switch legs down to your board. Print **two** caps ([§10](#10-case-print)).

### 6.8 SD card module — skip today

You do **not** have this part. Leave **D10–D13** empty. This program does not talk to an SD card yet, so the box still runs.

(You are also missing the **LM2596**. You do not need it for calls — use a 3.7V cell. See [§3.1](#31-get-calls-working-this-week-no-lm2596) and [§6.9](#69-sim800l--calls-from-a-37v-cell).)

When you buy the SD board, wire CS/MOSI/MISO/SCK to D10–D13, VCC per the print on the board, GND to GND. Format the card **FAT32**. SD logging is not in this program yet. Leave those pins empty until a later build adds it.

### 6.9 SIM800L — calls from a 3.7V cell

You **have** the phone chip. This is the mini phone. Antenna on. Nano-SIM in the slot (gold pads down, cut corner matching the drawing on the holder).

You **do not have** the LM2596. You **do not need it** for the pitch. Feed **VCC** from a **3.7V cell** ([§3.1](#31-get-calls-working-this-week-no-lm2596)). The Uno has **no** 4V pin.

| SIM800L | To | Today? |
|---------|-----|--------|
| VCC | Cell **plus** (18650 bump, or phone-battery **+**) | **Yes** — never Uno 5V or 3.3V |
| GND | Cell **minus** and Uno GND | **Yes** |
| TX | Uno **D5** (direct) | **Yes** |
| RX | Mid-point of the divider from **D6** | **Yes** |
| Antenna | Screw / uFL attached | **Yes** |

When the cell is on VCC, upload [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino). `PHONE_ALERTS` is already **true**. If you unplug the cell, set it back to **false** so Demo does not try to call.

If you later buy an LM2596: take the cell off VCC, set the screw to about **4.0V** on a meter, then SIM VCC → that output. Same GND rail.

**Divider.** The Uno talks too “loud” (5V). The phone chip listens at a quieter level. The resistors are a pressure drop.

**You did not get a 20kΩ.** Use two 10kΩ end to end as the ground leg (that equals 20k), plus one 10k from D6:

```
Uno D6  --- 10kΩ --------+--- SIM800L RX
                         |
                    10kΩ + 10kΩ
                    (two in a line = 20k)
                         |
                        GND
```

If you only have **two** 10kΩ (not three), use this. It is a little quieter (~2.5V) but still protects the chip:

```
Uno D6  --- 10kΩ ---+--- SIM800L RX
                    |
                  10kΩ
                    |
                   GND
```

The SIM RX wire always comes off the **middle**. Never a bare D6 wire to RX.

Keep `PHONE_ALERTS` as **true** (you have the 3.7V cell). Leave D10–D13 empty. There is no SD card in this program yet.

**Network light:** dark if VCC is empty. With the cell on: fast blink = hunting, **slow blink** = on the network.

**Caution:** CRITICAL and FIRE place a **real call** to `OWNER_CONTACT`. Warn the person who holds that phone.

### 6.10 Case

Print notes are in [§10](#10-case-print). Fit order that works:

1. Screw or seat the Uno on the standoffs. USB hole faces the cutout.
2. Seat the SIM board on its platform. Leave the **SD** platform and the **4V buck** platform empty.
3. Leave the extra empty shelf empty.
4. Route the LCD into the lid window. Seat LEDs in G / Y / R holes. Buttons through DEMO and RESET. Buzzer in its hole.
5. Clip the gas sensor in the MQ mount at the grill. Flame sensor toward the side window.
6. Antenna wire out the front slot. Close the lid. Four small screws.

---

## 7. Set phone numbers and upload the program

You load **one** file onto the Uno: [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino).

### 7.1 Install Arduino IDE

1. Download **Arduino IDE** from the Arduino website (the desktop app).
2. Plug the Uno with a **USB data cable**.
3. In the IDE: **Tools → Board → Arduino Uno**.
4. **Tools → Port** → pick the port that appears when the cable is in (on Windows it looks like COM3; on Mac/Linux it looks like `/dev/cu.usbserial-…` or `/dev/ttyUSB0`).
5. If the port never appears, the cable is often charge-only, or you need a CH340 driver for clone boards.

### 7.2 Library

**Sketch → Include Library → Manage Libraries.** Search **LiquidCrystal I2C** (by Frank de Brabander or a well-rated 1602 I2C library). Install it.

`SD` and `SoftwareSerial` are already built into the IDE. You do not install those.

### 7.3 Edit the three lines that are yours

Near the top of `aeroguard_x1-1.ino`:

```c
const char* OWNER_CONTACT = "+233557164067";
const char* SECONDARY_CONTACT = "+233599494342";
const char* DEVICE_LABEL = "AeroGuard Kitchen";
```

| Name | Put |
|------|-----|
| `OWNER_CONTACT` | Main / owner phone. Set to `+233557164067`. |
| `SECONDARY_CONTACT` | Backup phone. Set to `+233599494342`. Gets a text after the owner is warned (MEDIUM, CRITICAL, or FIRE). Not “go inside and fix a leak.” |
| `DEVICE_LABEL` | Short place name. It is printed on the LCD and inside texts. Example: `Hostel Block A Kitchen`. |

If the LCD is blank, change `LiquidCrystal_I2C lcd(0x27, 16, 2);` so `0x27` matches your backpack (`0x3F` is the other common value).

If the lighter test never hits FIRE, change `FLAME_DETECT_THRESHOLD` a little and try again.

Keep `PHONE_ALERTS` as **true** (cell on SIM VCC). Leave D10–D13 empty. There is no SD card in this program yet.

### 7.4 Upload

1. Click **Upload** (the arrow).
2. Wait until the IDE says **Done uploading**.
3. **Tools → Serial Monitor.** Set the speed box to **9600**.
4. The screen should jump to **READY SAFE** and **press DEMO D9**. Green light blinks. There is no waiting bar.

Do not upload any other `.ino` file for this demo.

---

## 8. How to test

Do this on a table. No open gas. Tell the owner-phone person a test call may come. Wire the 3.7V cell first, then upload with `PHONE_ALERTS` **true**.

| Check | What you do | What you should see |
|-------|-------------|---------------------|
| Boot | USB on. Cell on SIM VCC. | Screen **READY SAFE** and a **gas** number. Green blinks. |
| Live gas | Press **Reset** first. Do **not** press Demo. Hold an **unlit** lighter 10 cm from the gas sensor (the nose). Do not open the stove. | Gas number climbs. Then **LIVE** alarm: LOW → MEDIUM → CRITICAL. Same lights, beeps, texts, and calls as Demo. |
| Demo 1 | Press **Demo** once | **Green** LED. LCD **LOW**. Quiet. |
| Demo 2 | Press **Demo** again | **Yellow**. Short beeps. **Real SMS**. |
| Demo 3 | Press **Demo** again | **Red**. Loud alarm. **Phone rings**, then SMS. |
| Demo 4 | Press **Demo** again | Still red. LCD **FIRE**. Call + SMS. |
| Reset | Press **Reset** | Alarm off. Back to SAFE. |

- MEDIUM sends a **real text**.
- CRITICAL and FIRE place a **real call** (rings a bit, then hangs up) and a **real text**.
- Do not surprise a sleeping family member. Do not spam the fire service.

If SMS or call never arrives, look at the **screen words** and the **network light**:

| Screen / light | Meaning | What to do |
|----------------|---------|------------|
| Stays SAFE or LOW | Phone is not asked to send yet | Demo twice, to **MEDIUM** (yellow) |
| `phone waking` then `no SIM talk` | Chip did not hear AT | Cell on **VCC**, shared **GND**, TXD→D5, RXD←D6 through 10k. Not Uno 5V on VCC. |
| `wait network` then `no network` | Chip is on but not on a mast | Antenna on. SIM seated, PIN off, airtime. Light should go **slow blink**. SIM800L is **2G** only. |
| `texting owner` / `sms sent` | Box sent it | If the phone is still empty, check the number and that SMS is not blocked on the SIM |
| Fast blink forever | Still hunting | Move nearer a window. Try another 2G SIM. Cell may be sagging — use a charged 18650. |

Do not surprise a sleeping family member. Do not spam the fire service.

If Demo tries to call with no cell on VCC, set `PHONE_ALERTS` back to **false**.

There is **no SD log** until you buy the SD module.

---

## 9. Demo pitch script (~10 minutes)

Speak in this order. Short. Show the box in your hands.

1. **Problem.** LPG is in homes, hostels, and chop bars. If it leaks at night, people smell it late — or not at all.
2. **Who pays first.** Lead with **hostel wardens / multi-tenant housing**. Chop bars and family kitchens are the next wave.
3. **Show the product.** Cased unit. Green / yellow / red. One brain: the Arduino Uno. One phone chip: the SIM800L. This is the kit.
4. **Live demo.** Press **Demo**: LOW (green, quiet) → MEDIUM (yellow, beep, **text**) → CRITICAL (red, **the phone rings**). Press **Reset** to clear. The SIM is powered by a **3.7V cell** (18650 or old phone battery). The LM2596 and SD log can wait.
5. **App story.** Open the Vercel contest app on a phone. Pairing and status are **simulated** for judges. Smart vents live in the app. Real calls still come from the SIM in the box.
6. **Cost + market.** Be honest about parts cost. Who buys, who installs, who gets the SMS.
7. **Tough questions, ready answers.**
   - SD card = history for a demo, not a fire-proof vault.
   - Secondary number = a backup text after the owner is warned, not a first responder.
   - Fire service voice call is not in this kit (a silent ring with no address is not useful). Phase 2 can be a verified address text with a partner.
   - The website cannot drive the box. GSM does.

---

## 10. Case print

**Ready-to-print STL files (no software needed):** just drop these into your slicer.

| File | Print | Notes |
|------|-------|-------|
| [`aeroguard_x1_case_base.stl`](aeroguard_x1_case_base.stl) | ×1, flat | The tub that holds everything |
| [`aeroguard_x1_case_lid.stl`](aeroguard_x1_case_lid.stl) | ×1, flat (top face up) | Holds the two 6×6mm switches, LCD, LEDs, buzzer |
| [`aeroguard_x1_sensor_mount.stl`](aeroguard_x1_sensor_mount.stl) | ×1, flat | Clip for the MQ gas sensor |
| [`aeroguard_x1_button_cap.stl`](aeroguard_x1_button_cap.stl) | **×2**, flat | One press cap per switch (Demo + Reset) |

**Or print everything at once:** [`aeroguard_x1_case_all_in_one.stl`](aeroguard_x1_case_all_in_one.stl) puts all parts (base, lid, gas clip, and both button caps) on **one plate** so you can print them in a single job. It needs a bed about **220 × 220 mm** (an Ender-3 size). If your printer is smaller, use the four separate files above instead.

**Slice:** **PLA** or **PETG**, about **0.2 mm** layers, **20%** infill. No supports needed. In the all-in-one file the lid and caps are already flipped the right way up for a clean print (this means the lid's top text prints against the bed).

**Want to change something?** Edit the source [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) in [OpenSCAD](https://openscad.org/downloads.html) (free), pick a `part`, press **F6**, then **File → Export → Export as STL**.

| | This case |
|--|-----------|
| Outer size | **126 × 90 × 36 mm** |
| Look | Rounded shell |
| Outside | **AeroGuard** on lid + front; **X1** on the side |
| Inside | Uno standoffs; platforms for SIM, SD (later), 4V buck (later); four lid screws |
| Buttons | Two **6×6mm tactile switches** clip into the **lid**; a printed cap presses each one |

**Switches:** each 6×6mm tactile switch pushes up into the square pocket under the lid (a dab of glue holds it). Its plunger pokes through the round hole; the printed **button cap** drops in on top. Solder short wires from the switch legs down to your board.

This print is tuned to be friendly: thick walls, no tiny holes, and chunky, braced screw towers so it does not fail partway through.

---

## 11. Later — not this kit

A WiFi-board sketch named [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) sits in the repo for a future phase. **Do not buy, wire, or flash it for this demo.** The pitch product is Uno + SIM800L + the Vercel app.
