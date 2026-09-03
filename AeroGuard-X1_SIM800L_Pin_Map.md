# Your red SIM800L Coreboard — pin map only

This matches the **red board** with the metal SIM tray and two rows of 6 holes.

You will use **4 holes**. Leave the other 8 empty.

**There is no 4V hole on the Arduino Uno.** The Uno only has **5V** and **3.3V**. The SIM800L wants about **4.0V** from a **different** board: the **LM2596** (small module with a screw). That screw is how you “turn the pressure down” from 5V-ish to 4V.

---

## Top row (the only row you wire)

Looking at the board the same way as the photo (SIM tray in the middle, writing **SIM800L Coreboard** on the right):

Left → right:

| Hole on your board | Use it? | Goes to |
|--------------------|---------|---------|
| **NET** | No | Leave empty (network LED pad) |
| **VCC** | **Yes** | LM2596 **OUT**, about **4.0V**. Never Uno 5V. |
| **RST** | No | Leave empty |
| **RXD** | **Yes** | Uno **D6**, through the 10k resistors (not a bare wire) |
| **TXD** | **Yes** | Uno **D5**, straight jumper |
| **GND** | **Yes** | Uno GND and LM2596 GND (same drain) |

---

## Bottom row (leave all empty)

| Hole | What it is | Use it? |
|------|------------|---------|
| **RING** | “phone is ringing” pin | No |
| **DTR** | extra control | No |
| **MICP** / **MICN** | microphone | No |
| **SPKP** / **SPKN** | speaker | No |

You do not need a mic or speaker. The box still **calls and texts** with AT commands.

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

| What | Where it comes from |
|------|---------------------|
| Sensors, LCD, lights, buzzer | Uno **5V** |
| SIM800L **VCC** | **LM2596 OUT**, set to **~4.0V** |
| All grounds | Join together (Uno GND + LM2596 GND + SIM GND) |

**Do not** put SIM **VCC** on Uno 5V. **Do not** put it on Uno 3.3V. Both will fail or hurt the chip. The phone chip gulps a lot of current when it calls, like a pump kicking on.

1. USB power bank (or USB from the computer) feeds the Uno **and** the **IN** side of the LM2596.
2. Do **not** connect SIM **VCC** yet.
3. Meter the LM2596 **OUT** pads. Turn the screw until you see about **4.0V**.
4. Then SIM **VCC** → that **OUT**. SIM **GND** → common GND.

---

## SIM and antenna

- Slide the nano-SIM into the silver tray. Gold pads toward the board. Match the cut corner.
- The antenna is usually a **spring or small gold pad on the other side** of this board — not the **NET** hole. Screw / solder the antenna there.

Network LED (if your board has one): fast blink = hunting. Slow blink = on the network.
