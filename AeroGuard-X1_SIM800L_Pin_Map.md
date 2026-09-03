# Your red SIM800L Coreboard — pin map only

This matches the **red board** with the metal SIM tray and two rows of 6 holes.

You will use **3 holes today** (GND, RXD, TXD). Leave **VCC empty** until you have ~4V. Leave the other 8 empty.

**There is no 4V hole on the Arduino Uno.** The Uno only has **5V** and **3.3V**. The SIM800L wants about **4.0V** from a **different** board: the **LM2596** (small module with a screw). You **do not have** that board yet.

---

## Top row (the only row you wire)

Looking at the board the same way as the photo (SIM tray in the middle, writing **SIM800L Coreboard** on the right):

Left → right:

| Hole on your board | Use it today? | Goes to |
|--------------------|---------------|---------|
| **NET** | No | Leave empty (network LED pad) |
| **VCC** | **No — leave empty** | Needs ~4V from an LM2596 later, or a loose 18650 (~3.7V). Never Uno 5V. Never Uno 3.3V. |
| **RST** | No | Leave empty |
| **RXD** | **Yes** | Uno **D6**, through the 10k resistors (not a bare wire) |
| **TXD** | **Yes** | Uno **D5**, straight jumper |
| **GND** | **Yes** | Uno GND (same drain) |

---

## Bottom row (leave all empty)

| Hole | What it is | Use it? |
|------|------------|---------|
| **RING** | “phone is ringing” pin | No |
| **DTR** | extra control | No |
| **MICP** / **MICN** | microphone | No |
| **SPKP** / **SPKN** | speaker | No |

You do not need a mic or speaker. The box still **calls and texts** with AT commands — but only after VCC has ~4V.

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
| SIM800L **VCC** | **Leave empty.** No LM2596 on the table. |
| All grounds | Join together (Uno GND + SIM GND) |

**Do not** put SIM **VCC** on Uno 5V. **Do not** put it on Uno 3.3V. Both will fail or hurt the chip. The phone chip gulps a lot of current when it calls, like a pump kicking on.

**A resistor cannot replace the LM2596.** The 10k resistors are only for the D6 talk wire. They cannot make a steady ~4V for VCC. Leave VCC empty until you buy an LM2596 or use a loose 18650.

Keep `GSM_ENABLED` as **false** until VCC has ~4V. Lights, Demo, and Reset still work.

**When you later have ~4V**, pick one:

1. **Buy an LM2596** (best). USB bank feeds the Uno **and** the **IN** side of the LM2596. Meter the **OUT** pads. Turn the screw until you see about **4.0V**. Then SIM **VCC** → that **OUT**. Join LM2596 GND to the same drain.
2. **Or a single loose 18650** (~3.7V). Plus → SIM VCC. Minus → same GND as the Uno. Do **not** use a 5V boosted pack on VCC.

Then set `GSM_ENABLED` to **true** and upload the program again.

---

## SIM and antenna

- Slide the nano-SIM into the silver tray. Gold pads toward the board. Match the cut corner.
- The antenna is usually a **spring or small gold pad on the other side** of this board — not the **NET** hole. Screw / solder the antenna there.

Network LED (if your board has one): dark today (no VCC). Later: fast blink = hunting. Slow blink = on the network.
