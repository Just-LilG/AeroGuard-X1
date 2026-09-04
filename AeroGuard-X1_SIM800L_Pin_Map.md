# Your red SIM800L Coreboard — pin map only

**To test the chip alone** (no gas, no screen): upload [`sim800l_test/sim800l_test.ino`](sim800l_test/sim800l_test.ino). Serial Monitor **9600**. You want the word `OK`. Then type `t` for a test text, or `c` for a test call.

This matches the **red board** with the metal SIM tray and two rows of 6 holes.

You will use **4 holes** once a 3.7V cell is on: **VCC, GND, RXD, TXD**. Leave the other 8 empty.

**There is no 4V hole on the Arduino Uno.** The Uno only has **5V** and **3.3V**. You do **not** need the LM2596 this week. A **3.7V cell** (18650 or old phone battery) already sits in the chip’s safe window.

---

## Top row (the only row you wire)

Looking at the board the same way as the photo (SIM tray in the middle, writing **SIM800L Coreboard** on the right):

Left → right:

| Hole on your board | Use it today? | Goes to |
|--------------------|---------------|---------|
| **NET** | No | Leave empty (network LED pad) |
| **VCC** | **Yes** with a 3.7V cell | Cell **plus**. Never Uno 5V. Never Uno 3.3V. |
| **RST** | No | Leave empty |
| **RXD** | **Yes** | Uno **D6**, through the 10k resistors (not a bare wire) |
| **TXD** | **Yes** | Uno **D5**, straight jumper |
| **GND** | **Yes** | Cell minus + Uno GND (same drain) |

---

## Bottom row (leave all empty)

| Hole | What it is | Use it? |
|------|------------|---------|
| **RING** | “phone is ringing” pin | No |
| **DTR** | extra control | No |
| **MICP** / **MICN** | microphone | No |
| **SPKP** / **SPKN** | speaker | No |

You do not need a mic or speaker. The box **calls and texts** after VCC has a 3.7V cell and `PHONE_ALERTS` is true.

---

## The two talk wires, in one line

| Your board | Direction | Uno |
|------------|-----------|-----|
| **TXD** | chip talks → brain hears | **D5** straight |
| **RXD** | brain talks → chip hears | **D6** + resistors |

RXD is the quiet-down pin. You have no 20kΩ. Use two 10kΩ:

```
Uno D6 ---- 10kΩ ----+---- board RXD
                     |
                   10kΩ
                     |
                    GND
```

---

## Power — Uno has no 4V pin

The Uno is the brain. It drinks **5V** from USB. It cannot feed the SIM800L.

| What | Where it comes from **today** |
|------|-------------------------------|
| Sensors, LCD, lights, buzzer | Uno **5V** |
| SIM800L **VCC** | **3.7V cell plus** (18650 or old phone battery) |
| All grounds | Join together (Uno GND + cell minus + SIM GND) **with straight wires** |

**SIM GND is a fat drain. No resistor in that wire.**  
If the SIM light goes **off** when you add a “GND resistor,” that resistor is sitting in the power drain. Unplug it. Join SIM GND, cell minus, and Uno GND with a bare jumper.

The 10k to ground belongs **only** on the D6 / RXD talk joint, as a tiny side pipe. Not in the SIM **GND** hole as the only ground. Not on **VCC**.

**Do not** put SIM **VCC** on Uno 5V. **Do not** put it on Uno 3.3V. Both will fail or hurt the chip. The phone chip gulps a lot of current when it calls, like a pump kicking on.

**A resistor cannot replace the cell.** The 10k resistors are only for the D6 talk wire.

Keep `PHONE_ALERTS` as **true**. You have the 3.7V cell. Lights, Demo, and Reset still work. If you unplug the cell, set it **false**.

```
Cell PLUS (+)  ----  board VCC
Cell MINUS (−) ----  board GND + Uno GND
```

18650 bump = plus. Flat end = minus. Tape the sides. Skip a puffy cell. Uno still drinks 5V from USB. Do not use a 5V boost/USB hole on VCC. Do not open a sealed power bank.

The LM2596 can wait. If you buy one later, take the cell off VCC first, meter ~4.0V, then use that output.

---

## SIM and antenna

- Slide the nano-SIM into the silver tray. Gold pads toward the board. Match the cut corner.
- The antenna is usually a **spring or small gold pad on the other side** of this board — not the **NET** hole. Screw / solder the antenna there.

Network LED (if your board has one): dark today (no VCC). Later: fast blink = hunting. Slow blink = on the network.
