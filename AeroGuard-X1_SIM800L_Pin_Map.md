# SIM800L only — pin map

This page is **only** the phone chip. Nothing else.

The SIM800L is a mini phone. The Arduino Uno is the brain. They talk on **two wires** (D5 and D6). Power is a **separate 4V tap**, not the Uno 5V pin.

---

## What to ignore on the SIM800L board

Leave these **unconnected** unless a print on your board says you must:

| Pad / pin | Do this |
|-----------|---------|
| RST | Leave empty |
| RING | Leave empty |
| DTR | Leave empty |
| GND (extra pads) | Only need **one** GND to the common ground |
| 5V | **Do not use** if your board has it |

You only use: **VCC, GND, TX, RX, antenna, SIM slot**.

---

## Power first (before any data wire)

1. Disconnect SIM **VCC**.
2. Power the **LM2596** (the little board with a screw).
3. Measure the output. Turn the screw until the meter says about **4.0V**.
4. Then connect SIM **VCC**.

| SIM800L pin | Goes to | Notes |
|-------------|---------|--------|
| **VCC** (sometimes VIN) | LM2596 **OUT** (~4.0V) | **Never** Uno 5V. That pin is too weak. The chip gulps current when it calls. |
| **GND** | Uno GND **and** buck GND | One shared drain. Join them. |

---

## Talk wires (only two)

TX means “this chip talks.” RX means “this chip listens.”

| SIM800L pin | Goes to | How |
|-------------|---------|-----|
| **TX** | Uno **D5** | Straight jumper. No resistor. The chip talks quietly enough. |
| **RX** | Uno **D6** | **Not** a straight jumper. Must go through resistors (below). |

Remember it as a conversation:

- SIM **TX** → Uno **D5** = the phone chip speaks, the Uno hears  
- Uno **D6** → SIM **RX** = the Uno speaks, the phone chip hears (voice must be turned down)

---

## D6 resistors (you have no 20kΩ)

Do **not** put a bare wire from D6 to RX. 5V will shout at a 3V ear.

**If you have two 10kΩ resistors** (simplest):

| From | Through | To |
|------|---------|-----|
| Uno **D6** | one **10kΩ** | the **middle** (this middle also goes to SIM **RX**) |
| that same **middle** | the other **10kΩ** | **GND** |

```
Uno D6 ---- 10kΩ ----+---- SIM800L RX
                     |
                   10kΩ
                     |
                    GND
```

**If you have three 10kΩ** (closer to the original 10k+20k plan):

```
Uno D6 ---- 10kΩ --------+---- SIM800L RX
                         |
                    10kΩ + 10kΩ  (two in a line)
                         |
                        GND
```

The SIM **RX** wire always comes off the **+** in the middle.

---

## Antenna and SIM card

| Part | What to do |
|------|------------|
| Antenna | Screw it on (or clip the uFL cable). No antenna = no network. |
| Nano-SIM | Slide in with **gold pads down**. Match the cut corner to the drawing on the holder. Needs call + SMS credit. |

---

## Uno side (same two pins)

| Uno pin | Job |
|---------|-----|
| **D5** | Hears the SIM800L (from SIM TX) |
| **D6** | Talks to the SIM800L (to SIM RX, through the resistors) |
| **GND** | Shared with the SIM800L |

Leave **D10–D13** empty (no SD board). Leave **A1** and **A3** empty.

---

## Is it alive?

On many SIM800L boards the network LED:

- **fast blink** = still hunting  
- **slow blink** = on the network — good  

Owner number in the program: `+233557164067`. Backup: `+233508705321`. Demo MEDIUM texts. CRITICAL / FIRE calls.
