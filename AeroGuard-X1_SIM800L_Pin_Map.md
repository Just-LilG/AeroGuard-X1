# Your red SIM800L Coreboard — pin map only

This matches the **red board** with the metal SIM tray and two rows of 6 holes.

You will use **4 holes**. Leave the other 8 empty.

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

## Power order

1. Do **not** connect **VCC** yet.
2. Set the LM2596 screw to about **4.0V** on a meter.
3. Then **VCC** → that 4V. **GND** → common ground.
4. Then **TXD** and **RXD**.

---

## SIM and antenna

- Slide the nano-SIM into the silver tray. Gold pads toward the board. Match the cut corner.
- The antenna is usually a **spring or small gold pad on the other side** of this board — not the **NET** hole. Screw / solder the antenna there.

Network LED (if your board has one): fast blink = hunting. Slow blink = on the network.
