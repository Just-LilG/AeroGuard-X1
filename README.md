# AeroGuard-X1

A small box that watches for **LPG leaks and fire**, then **calls and texts** the owner.

**The program you upload:** [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino)  
USB **data** cable. Board = **Arduino Uno**. Serial Monitor **9600**.

**Pictures:** [`AeroGuard-X1_Assembly_Guide.html`](AeroGuard-X1_Assembly_Guide.html) · SIM board: [`AeroGuard-X1_SIM800L_Pin_Map.html`](AeroGuard-X1_SIM800L_Pin_Map.html)  
**Wires:** [`AeroGuard-X1_Pin_Map_Wiring_Reference.md`](AeroGuard-X1_Pin_Map_Wiring_Reference.md) · full story: [`AeroGuard-X1_Build_Guide.md`](AeroGuard-X1_Build_Guide.md)

## Take this to the pitch

1. Upload **only** `aeroguard_x1-1.ino`. Not the SIM-only test. Not the ESP32 file.
2. Screen says **READY SAFE** and a **gas** number. Green light blinks.
3. **Demo** = D9 to GND. Each press: LOW (green) → MEDIUM (yellow + beep) → CRITICAL (red) → FIRE. **Reset** = D7 to GND.
4. Real gas on the MQ nose (A0) can raise the same alarm with no Demo. Unlit lighter, far from the stove. Do not open the gas valve.
5. Show the Vercel app on a phone. That app is a **poster for judges**. It does not talk to the box.
6. Skip ESP32. Skip SD (leave D10–D13 empty). Skip LM2596. Cell plus → SIM **VCC**. Never Uno 5V on VCC. SIM **GND** is a **straight** wire — no 10k in that hole.
7. If the SIM light **blinks**, MEDIUM can text and CRITICAL can call. If the light is **dark**, still show lights and Demo. Fix the phone chip later.

**Phones in the program**
- Owner: `+233557164067`
- Backup: `+233599494342`

## Same pin map (do not change)

| Uno | Goes to |
|-----|---------|
| A0 | Gas analog |
| A2 | Flame analog |
| A4 | LCD SDA |
| A5 | LCD SCL |
| D2 / D3 / D4 | Green / yellow / red |
| D5 | SIM **TXD** straight |
| D6 | SIM **RXD** through 10k (the extra 10k is a side pipe to the GND *rail*, not the only wire in SIM GND) |
| D7 | Reset → GND |
| D8 | Buzzer |
| D9 | Demo → GND |
| A1, A3, D10–D13 | Empty |

## Files

| Path | Role |
|------|------|
| [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino) | **Upload this** to the Uno |
| [`AeroGuard-X1_SIM800L_Pin_Map.html`](AeroGuard-X1_SIM800L_Pin_Map.html) | Picture of the red phone chip |
| [`companion/`](companion/) | Contest phone UI |
| [`aeroguard_x1_case_all_in_one.stl`](aeroguard_x1_case_all_in_one.stl) | Case, one print |

```bash
cd companion
npm install
npm run dev -- --hostname 127.0.0.1 --port 43123
```
