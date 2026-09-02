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

You **have** parts **1–7** and **11–16**. You **do not have** the **SIM800L phone chip** (#8) or the **SD card module** (#10). You also still need the **LM2596** (#9) before the phone chip can be powered.

Wire the nose, eye, screen, lights, beeper, and buttons now. Leave D5, D6, and D10–D13 empty. Full pin tables: [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md).

| # | Role | What to buy | Qty | On the table? | Plain words |
|---|------|-------------|-----|---------------|-------------|
| 1 | Brain | **Arduino Uno R3** (ATmega328P). Clone with CH340 chip is OK. | 1 | **Have** | The small computer. All other parts plug into it. |
| 2 | Gas nose | **MQ-5 LPG Gas Sensor Module** (or **MQ-2** if MQ-5 is hard to find). Breakout with **AO** and **DO** pins. | 1 | **Have** | Sniffs LPG / smoke mix. Prefer MQ-5 for cooking gas. |
| 3 | Flame eye | **IR Flame Sensor Module** (often **KY-026**) | 1 | **Have** | Sees a flame’s infrared light. Use the **analog (A0)** pin on the module. |
| 4 | Screen | **LCD 1602 I2C** — 16 characters × 2 lines with a **PCF8574** backpack | 1 | **Have** | The little green/blue text screen. I2C means only two data wires. |
| 5 | Lights | **5mm LED**: 1 green, 1 yellow, 1 red + three **220Ω** resistors | 1 set | **Have** | Stage lights. Resistors are tiny “speed bumps” so the LEDs do not burn. |
| 6 | Alarm | **Active buzzer module** (often **KY-012**) | 1 | **Have** | The beeper. |
| 7 | Buttons | Two **6×6mm tactile push buttons**, or two **KY-004** button modules | 2 | **Have** | **Demo** and **Reset**. Momentary = click and it springs back. |
| 8 | Phone chip | **SIM800L V2.0 GSM/GPRS module** with antenna | 1 | **Buy later** | Sends SMS and makes calls. Needs strong ~4V power. |
| 9 | SIM power | **LM2596 DC-DC buck converter** | 1 | **Buy later** | A “pressure reducer” for electricity. You set the output to about **4.0V**. Buy this with the phone chip. |
| 10 | Log | **Micro SD card module (SPI)** (often **HW-125**) + a **microSD** card formatted **FAT32** | 1 | **Buy later** | Writes a simple event file. Not a fire-proof vault. |
| 11 | Battery | **5V USB power bank**, *or* **18650** cell + **TP4056** charger + boost to 5V | 1 | **Have** | Demo power. USB bank is the easy path. |
| 12 | Wiring | **MB-102 breadboard** + **Dupont jumper wires** (male-male, male-female) | 1 set | **Have** | Breadboard = plastic hole grid so you can plug wires without soldering. |
| 13 | SIM protect | **10kΩ** resistor + **20kΩ** resistor | 1 pair | Bag until #8 | Voltage divider. Drops the Uno’s 5V talk-line so the SIM chip is not hurt. |
| 14 | Network | **Nano-SIM** with call + SMS credit | 1 | Bag until #8 | Must fit the SIM800L slot. Use an adapter if your SIM is larger. |
| 15 | Case | Printed **AeroGuard-X1** case from [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad) | 1 | **Have** | Lid, base, sensor clip. See [§10](#10-case-print). |
| 16 | Tools | USB **data** cable (not charge-only), small screwdriver, **multimeter** | 1 | **Have** | Multimeter = the meter that reads volts. You need it to set 4.0V later. |

**Optional, not required:** extra MQ-5 for a “second kitchen” story later.

**Do not buy for this kit:** a servo motor, a vent flap, a Bluetooth dongle, or a second brain board.

**Shop tip:** if a listing says “MQ-2 smoke sensor,” it still sees LPG mixed with smoke. For a cleaner cooking-gas story, ask for **MQ-5**.

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
| D5 | SIM800L **TX** | **No** — no phone chip yet |
| D6 | SIM800L **RX** | **No** — no phone chip yet |
| D7 | Reset button, other leg to GND | **Yes** |
| D8 | Buzzer | **Yes** |
| D9 | Demo button, other leg to GND | **Yes** |
| D10 | SD **CS** | **No** — no SD module yet |
| D11 | SD **MOSI** | **No** |
| D12 | SD **MISO** | **No** |
| D13 | SD **SCK** | **No** |
| 5V | Sensors, LCD, LEDs, buzzer, buttons | **Yes** |
| GND | **Every** module you plug in | **Yes** |
| ~4V from buck | **SIM800L VCC only** | **Later** |

A1 and A3 stay empty. D5, D6, and D10–D13 stay empty until those parts arrive.

---

## 5. Power warnings (read before any power)

1. **The SIM800L is thirsty.** When it calls, it gulps current (around **2A** peaks). Feed it from the **LM2596** set to about **4.0 volts** (3.7–4.2V is the safe window). **Never** take SIM power from the Uno **5V** pin. That pin is a thin hose. The chip will reset or die.
2. **Measure before you connect the SIM.** Power the buck. Turn the small screw. Read the output with a multimeter. Only then clip SIM VCC to that 4V.
3. **Common ground.** Uno GND, battery GND, buck GND, SIM GND, and every sensor GND must join. Same drain for every pipe. If grounds do not meet, modules misbehave or the SIM never talks.
4. **Voltage divider on D6.** The Uno speaks at 5V. The SIM800L listen pin wants about 2.8V. Use **10kΩ** from D6 to the SIM RX pin, and **20kΩ** from that same SIM RX pin down to GND. Do not run a bare wire from D6 to SIM RX.
5. **SIM TX to D5 is direct.** The chip speaks at ~3.3V. The Uno can hear that. No divider on that wire.
6. **One USB data cable** for the computer. Charge-only cables look the same and will not upload the program.
7. **Set owner phone numbers in the program** before you demo a real call. See [§7](#7-set-phone-numbers-and-upload-the-program).

---

## 6. Step-by-step assembly

Do these in order. Unplug USB and batteries while you move wires. After each step you can plug USB in to check the screen and lights.

**Today, stop after the buttons (step 6.7).** Skip the SD module and the phone chip. They are not on the table.

Open the HTML guide beside this page if you want the same steps with bigger cards. Or use the pin sheet: [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md).

### 6.1 Power rails

**Goal:** a 5V strip for the small parts, and one shared GND. No 4V SIM rail until you buy the LM2596 and SIM800L.

1. Put the Uno on the table. Note **5V**, **GND**, and the numbered pins.
2. Run a red jumper from Uno **5V** to the breadboard **+** rail.
3. Run a black jumper from Uno **GND** to the breadboard **−** rail.
4. Power the Uno from USB (computer or 5V power bank).
5. When the buck and phone chip arrive: sit the **LM2596** on the side. Input from the 5V bank. Output ~4V for **SIM VCC only**. Join buck **GND** to the same **−** rail. **Do not** feed the buck output into the Uno 5V pin.

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

Do not wire buttons to 5V. Label the lid **RESET** and **DEMO** so you do not mix them in front of judges.

### 6.8 SD card module — skip today

You do **not** have this part. Leave **D10–D13** empty. The program still runs. Serial will say `No SD module (OK for this bench).`

When you buy it, wire CS/MOSI/MISO/SCK to D10–D13, VCC per the print on the board, GND to GND. Format the card **FAT32**. The program writes `gaslog.txt`. It is **not** a fire-proof black box.

### 6.9 SIM800L + 4V buck — skip today

You do **not** have the phone chip. Leave **D5** and **D6** empty. Keep `GSM_ENABLED` as **false** in [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) so Demo will not try to call (that wait is about 18 seconds and would freeze the demo). Keep `SD_ENABLED` **false** too.

When the **SIM800L**, **LM2596**, antenna, SIM, and 10k/20k pair are all on the table, follow the pin sheet § “Leave empty until you buy” and then:

1. Disconnect SIM VCC.
2. Power the LM2596 input.
3. Meter the output pads. Turn the screw until you see about **4.0V**.
4. Connect SIM **VCC** to that output. Never Uno 5V.
5. TX → D5. RX → divider from D6. Set `GSM_ENABLED` to **true**. Re-upload.

**Caution later:** CRITICAL and FIRE place a **real call** to `OWNER_CONTACT`.

### 6.10 Case

Print notes are in [§10](#10-case-print). Fit order that works:

1. Screw or seat the Uno on the standoffs. USB hole faces the cutout.
2. Seat SIM, SD, and 4V buck on their labeled platforms **when those parts arrive**. Today those shelves stay empty.
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
const char* OWNER_CONTACT = "+233XXXXXXXXX";
const char* SECONDARY_CONTACT = "+233YYYYYYYYY";
const char* DEVICE_LABEL = "AeroGuard Kitchen";
```

| Name | Put |
|------|-----|
| `OWNER_CONTACT` | The owner’s number in international form, like `+23324…` |
| `SECONDARY_CONTACT` | Roommate or landlord. Gets an SMS only if CRITICAL/FIRE is still on after **3 minutes**. Backup check-in. Not “go inside and fix a leak.” |
| `DEVICE_LABEL` | Short place name. It is printed on the LCD and inside texts. Example: `Hostel Block A Kitchen`. |

If the LCD is blank, change `LiquidCrystal_I2C lcd(0x27, 16, 2);` so `0x27` matches your backpack (`0x3F` is the other common value).

If the lighter test never hits FIRE, change `FLAME_DETECT_THRESHOLD` a little and try again.

Keep `GSM_ENABLED` as `false` until the SIM800L and 4V buck are wired. Keep `SD_ENABLED` as `false` until the SD module is wired. Then change them to `true` and upload again.

### 7.4 Upload

1. Click **Upload** (the arrow).
2. Wait until the IDE says **Done uploading**.
3. **Tools → Serial Monitor.** Set the speed box to **9600**.
4. You should see `AeroGuard-X1` / `Ready`. The LCD shows **Starting…** then **Calibrating…** for **45 seconds**. Do not press Demo during that wait. It is learning quiet air, like tasting the river before the rain.

Do not upload any other `.ino` file for this demo.

---

## 8. How to test

Do this on a table. No open gas. Tell the owner-phone person that a test call may come.

| Check | What you do | What you should see **today** (no phone chip, no SD) |
|-------|-------------|---------------------|
| Boot | Power USB after upload | LCD: Starting → Calibrating (~45s) → live SAFE / label. Serial: `GSM off` and `No SD module`. |
| Serial | Monitor at 9600 | Lines like `STATUS level=SAFE …` once per second |
| Demo 1 | Press **Demo** once | **Green** LED. LCD **LOW**. Quiet (no buzzer). |
| Demo 2 | Press **Demo** again | **Yellow** LED. Short beeps. **No SMS** until the phone chip is fitted. |
| Demo 3 | Press **Demo** again | **Red** LED. Loud alarm. **No call** yet. |
| Demo 4 | Press **Demo** again | Still **red**. LCD **FIRE RISK** / **DEMO MODE**. No call yet. |
| Reset | Press **Reset** | Alarm stops. Demo ends. Calibrating ~45s. Back to SAFE. |

**When the SIM800L is on the box** (later): MEDIUM sends a **real text**. CRITICAL and FIRE place a **real call**. Warn the owner-phone person. Put your own number in first if you are alone.

---

## 9. Demo pitch script (~10 minutes)

Speak in this order. Short. Show the box in your hands.

1. **Problem.** LPG is in homes, hostels, and chop bars. If it leaks at night, people smell it late — or not at all.
2. **Who pays first.** Lead with **hostel wardens / multi-tenant housing**. Chop bars and family kitchens are the next wave.
3. **Show the product.** Cased unit. Green / yellow / red. One brain: the Arduino Uno. One phone chip: the SIM800L. This is the kit.
4. **Live demo.** Press **Demo**: LOW (green, quiet) → MEDIUM (yellow, beep) → CRITICAL (red, loud). Press **Reset** to clear. **Today there is no real call** — the phone chip is not on the table yet. Say that to judges. Lights and sound still prove the stages.
5. **App story.** Open the Vercel contest app on a phone. Pairing and status are **simulated** for judges. Smart vents live in the app. Real calls still come from the SIM in the box.
6. **Cost + market.** Be honest about parts cost. Who buys, who installs, who gets the SMS.
7. **Tough questions, ready answers.**
   - SD card = history for a demo, not a fire-proof vault.
   - Secondary number = a delayed SMS check-in, not a first responder.
   - Fire service voice call is not in this kit (a silent ring with no address is not useful). Phase 2 can be a verified address text with a partner.
   - The website cannot drive the box. GSM does.

---

## 10. Case print

File: [`aeroguard_x1_case.scad`](aeroguard_x1_case.scad).

1. Install [OpenSCAD](https://openscad.org/downloads.html) (free 3D-shape software).
2. Open the file. Leave `part = "all_export";`.
3. Press **F6** (render). **File → Export → Export as STL**.
4. Slice in your printer app. **PLA** or **PETG**. About **0.2 mm** layers, **20%** infill. Print **base** flat, **lid** flat, **sensor mount** flat.

| | This case |
|--|-----------|
| Outer size | **126 × 90 × 36 mm** |
| Look | Rounded shell |
| Outside | **AeroGuard** on lid + front; **X1** on the side |
| Inside | Uno standoffs; platforms for SIM, SD, 4V buck; four lid screws |
| Extra shelf | Leave **empty** |

You get **base**, **lid**, and **sensor mount** (a spare mount is on the plate for the multi-zone story). No vent-flap parts.

---

## 11. Later — not this kit

A WiFi-board sketch named [`esp32_aeroguard_bridge.ino`](esp32_aeroguard_bridge.ino) sits in the repo for a future phase. **Do not buy, wire, or flash it for this demo.** The pitch product is Uno + SIM800L + the Vercel app.
