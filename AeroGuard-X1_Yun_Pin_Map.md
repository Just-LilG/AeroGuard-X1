# AeroGuard-X1 — Arduino Yún pin map (brain)

The **Yún replaces the Uno and the ESP32**: sensors, LCD, LEDs, buttons, SD, and GSM on the 32U4; **Wi‑Fi is on-board Linux**. Do not wire the ESP32 on this path.

Arduino IDE: **Tools → Board → Arduino Yún** (or Yún Rev2), then upload [`aeroguard_x1-1.ino`](aeroguard_x1-1.ino). The sketch picks Yún pins automatically.

**Leave D0 and D1 empty.** Those are `Serial1` to Linux (Bridge). Using them for SIM800L or an ESP32 will break Yún Wi‑Fi.

**SIM800L:** still wait for the **LM2596 ~4.0 V** (part 9). Same as the Uno pack.

**Case:** the printed v2 shell is sized for **Uno + ESP32**. The Yún is a bit longer and has Ethernet/USB in different places — use it on the **breadboard** first; reprint later if you want it in the box.

---

## Full pin map (Yún)

| Pin | Function | Notes |
|-----|----------|--------|
| **D0 / D1** | Linux / Bridge | **Do not wire** |
| D2 | LCD SDA | Hardware I2C (not A4) |
| D3 | LCD SCL | Hardware I2C (not A5) |
| D4 | Green LED (LOW) | 220Ω on anode |
| D5 | Yellow LED (MEDIUM) | |
| D6 | Red LED (CRITICAL / FIRE) | |
| D7 | Buzzer | |
| D8 | Reset → GND | `INPUT_PULLUP` |
| D9 | Demo → GND | `INPUT_PULLUP` |
| D10 | SD **CS** | |
| D11 | SIM800L TX → Yún RX | SoftwareSerial — **later** (4 V) |
| D12 | Yún TX → SIM800L RX | Via 10k/20k — **later** |
| D13 | On-board LED | Leave free |
| **ICSP MOSI** | SD MOSI | Not the D11 header pin |
| **ICSP MISO** | SD MISO | Not D12 header |
| **ICSP SCK** | SD SCK | Not D13 header |
| A0 | Gas analog | |
| A2 | Flame analog (module pin A0) | Same gotcha as Uno |
| A4 / A5 | Free | Not I2C on Yún |
| 5V / GND | Modules | ESP32 USB not required |
| ~4 V | SIM VCC only | After buck is set |

---

## Module wires (same modules as the current pack)

**LCD backpack:** GND, VCC→5V, **SDA→D2**, **SCL→D3**.

**SD:** 5V, GND, CS→D10, **MOSI/MISO/SCK on the 2×3 ICSP header** (match the SD module labels to ICSP, not to D11–D13).

**ESP32 divider (part 15):** unused on this path. Keep the pair for **SIM RX (D12)** when the buck arrives.

**Wi‑Fi / app:** Yún Linux must finish booting (often **1–2 minutes**). USB Serial 9600 still prints `STATUS` immediately. Phone HTTP like the ESP32 `/status` is a **follow-on** (Bridge / `arduino.local`) — not in this sketch yet. Contest companion can stay simulated.

---

## Why this is not the Uno map

| Uno | Yún |
|-----|-----|
| I2C on A4/A5 | I2C on **D2/D3** |
| LEDs D2/D3/D4 | LEDs **D4/D5/D6** |
| GSM D5/D6 | GSM **D11/D12** |
| App UART A1/A3 + ESP32 | **No ESP32**; D0/D1 reserved |
| SD SPI on D11–D13 | SD SPI on **ICSP** + CS D10 |

Uno wiring docs stay valid if you keep the Uno as brain.
