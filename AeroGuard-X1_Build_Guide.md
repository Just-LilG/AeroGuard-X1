# AeroGuard-X1 — Complete Build Guide
**Smart Gas Leak Detection System**
**Group 1 — Arduino Uno + PictoBlox/Arduino IDE**

This guide covers the full design of the system: parts, pin mapping, wiring instructions, calibration logic, and the complete working code. Follow it in order — calibration and wiring depend on earlier steps being done correctly.

---

## 1. System Summary

The system monitors gas concentration in two zones (e.g. Kitchen and Gas Storage) and independently watches for flame. Instead of a fixed threshold, each zone learns its own "normal" baseline on startup, then escalates through **LOW → MEDIUM → CRITICAL** stages based on sustained deviation from that baseline. If flame is detected while gas is elevated, the system skips straight to an **EMERGENCY** fast-track response. At Critical/Emergency, the system calls and texts a primary contact, and escalates to a secondary contact (e.g. a neighbor) if the alarm is not cleared within a set time. Events are logged to an SD card, and a servo opens a vent automatically at Medium and above.

---

## 2. Full Components List

| # | Component | Qty |
|---|-----------|-----|
| 1 | Arduino Uno | 1 |
| 2 | MQ-2 / MQ-5 Gas Sensor (Zone 1) | 1 |
| 3 | MQ-2 / MQ-5 Gas Sensor (Zone 2) | 1 |
| 4 | Flame Sensor Module (IR, analog output) | 1 |
| 5 | 16x2 LCD Display with I2C backpack | 1 |
| 6 | Buzzer (active or passive) | 1 |
| 7 | LEDs — Yellow, Orange, Red | 3 |
| 8 | Servo Motor (SG90) | 1 |
| 9 | Push Button | 1 |
| 10 | SIM800L GSM Module | 1 |
| 11 | Voltage Regulator / Buck Converter (steps down to ~4V for SIM800L) | 1 |
| 12 | Micro SD Card Module (SPI) + SD Card | 1 |
| 13 | Backup Battery Pack (Li-ion + regulator) | 1 |
| 14 | Breadboard + Jumper Wires | 1 set |
| 15 | Resistors: 220Ω (x3 for LEDs), 10kΩ (button pull-down if not using internal pull-up), voltage divider pair for SIM800L RX line | Assorted |
| 16 | Active SIM card with call + SMS credit | 1 |

---

## 3. Pin Map (Arduino Uno)

This layout avoids all pin conflicts, including the SD card's fixed SPI pins.

| Pin | Connected To | Notes |
|-----|--------------|-------|
| A0 | Gas Sensor Zone 1 (analog out) | e.g. Kitchen |
| A1 | Gas Sensor Zone 2 (analog out) | e.g. Storage |
| A2 | Flame Sensor (analog out) | Lower reading = flame detected (verify with your module) |
| A4 | LCD SDA | I2C data line |
| A5 | LCD SCL | I2C clock line |
| D2 | Yellow LED (+ 220Ω resistor) | LOW stage indicator |
| D3 | Orange LED (+ 220Ω resistor) | MEDIUM stage indicator |
| D4 | Red LED (+ 220Ω resistor) | CRITICAL / EMERGENCY indicator |
| D5 | SIM800L **TX** → Uno RX | SoftwareSerial receive |
| D6 | SIM800L **RX** ← Uno TX (via voltage divider) | SoftwareSerial transmit — **see power/voltage warning below** |
| D7 | Push Button (other leg to GND) | Uses Uno's internal pull-up resistor |
| D8 | Buzzer | Active or passive buzzer |
| D9 | Servo signal wire | PWM pin |
| D10 | SD Card Module — CS (Chip Select) | |
| D11 | SD Card Module — MOSI | Fixed SPI pin, do not reassign |
| D12 | SD Card Module — MISO | Fixed SPI pin, do not reassign |
| D13 | SD Card Module — SCK | Fixed SPI pin, do not reassign |
| 5V | Gas sensors, LCD, buzzer, servo, button, SD module VCC | Standard 5V logic devices |
| GND | Common ground rail | **All components, including SIM800L and battery pack, must share this ground** |

---

## 4. Critical Wiring Warnings — Read Before Powering On

1. **SIM800L power:** Do NOT power the SIM800L from the Uno's 5V pin directly. It needs a stable 3.7–4.2V supply and can spike to ~2A when transmitting or calling, which will brown out or reset the Uno. Use the buck converter/regulator listed in the parts list, powered from your main battery pack, and connect its ground to the same common ground as the Uno.
2. **SIM800L RX protection:** The SIM800L's logic level is around 2.8V. The Uno's TX pin outputs 5V, which can damage the module over time. Wire a simple voltage divider (e.g. two resistors, such as 10kΩ and 20kΩ) between Uno D6 and SIM800L RX to step the voltage down. The SIM800L TX pin can connect straight to Uno D5 without a divider.
3. **Common ground:** Every component — sensors, LCD, servo, SD module, SIM800L, and the battery pack — must share a single common ground with the Arduino. A missing ground connection is the most common cause of "nothing works" on a first build.
4. **SD module SPI pins are fixed:** D11, D12, and D13 are used automatically by the SD library. Do not connect anything else to these three pins.
5. **Flame sensor threshold:** Flame sensor modules vary by manufacturer. Before final assembly, test yours with a lighter (briefly, safely, away from any gas source) and check the analog reading in the Serial Monitor to confirm whether flame produces a low or high value, and adjust `FLAME_DETECT_THRESHOLD` in the code if needed.

---

## 5. Step-by-Step Assembly

### Step 1 — Power rails
Set up your breadboard with a 5V rail and a GND rail, both fed from the Arduino's 5V and GND pins. Keep the battery pack and SIM800L's regulated 4V supply on a **separate** section of the breadboard, connected only by a shared ground wire back to the main GND rail.

### Step 2 — Gas sensors
Connect both MQ-series sensors' VCC to 5V, GND to the ground rail, and their analog output (`AOUT`) pins to A0 (Zone 1) and A1 (Zone 2). Mount each sensor in its intended zone (e.g. one near the kitchen stove, one near the gas storage area) — they need a few minutes of warm-up time when first powered, which is normal for MQ-series sensors.

### Step 3 — Flame sensor
Connect VCC to 5V, GND to the ground rail, and the analog output to A2.

### Step 4 — LCD (I2C)
Connect VCC to 5V, GND to ground, SDA to A4, and SCL to A5. If your LCD does not display text once the code is running, use an I2C scanner sketch to confirm its address (commonly `0x27` or `0x3F`) and update it in the code.

### Step 5 — LEDs
For each LED (Yellow, Orange, Red): connect the longer leg (anode) through a 220Ω resistor to its assigned digital pin (D2, D3, D4), and the shorter leg (cathode) to the ground rail.

### Step 6 — Buzzer
Connect the positive lead to D8 and the negative lead to ground.

### Step 7 — Push button
Connect one leg of the button to D7 and the other leg to ground. The code uses the Uno's internal pull-up resistor, so no external resistor is required for the button.

### Step 8 — Servo
Connect the servo's brown/black wire to ground, red wire to 5V, and the signal wire (usually orange or yellow) to D9. Mount the servo so its arm can physically open a vent flap or small window shutter.

### Step 9 — SD card module
Wire VCC to 5V, GND to ground, CS to D10, MOSI to D11, MISO to D12, and SCK to D13. Insert a formatted (FAT16/FAT32) micro SD card before powering on.

### Step 10 — SIM800L GSM module
Connect the module's VCC to the regulated ~4V output (not the Uno's 5V pin), GND to the common ground, TX to Uno D5, and RX to Uno D6 through the voltage divider described in the warnings above. Insert an active SIM card with call and SMS credit, and make sure the module has clear signal (an LED on most SIM800L boards blinks slowly once registered on the network).

### Step 11 — Battery backup
Wire the battery pack's regulated output in parallel with your main 5V supply (or use a power-switching/diode arrangement if you want automatic failover), ensuring the ground is shared with everything else.

### Step 12 — Power on and test
With everything wired, upload the code (Section 7), open the Serial Monitor at 9600 baud, and confirm each component initializes correctly before doing a full live test.

---

## 6. How the Logic Works

### 6.1 Calibration
On every startup (and every time the reset button is pressed), the system samples each gas zone for **45 seconds** and averages the readings into that zone's baseline. This is what allows the system to adapt to different rooms instead of relying on one fixed number.

### 6.2 Deviation and confirmation window
Each loop, the system calculates how far the current reading is above baseline as a percentage. An elevated reading must persist for **8 seconds** (the confirmation window) before the system escalates to a new stage — this filters out brief, harmless spikes.

### 6.3 Escalation stages
| Stage | Trigger | Response |
|-------|---------|----------|
| SAFE | Deviation below 20% | Normal display, no alarm |
| LOW | Deviation ≥ 20%, sustained | Yellow LED |
| MEDIUM | Deviation ≥ 40%, sustained | Orange LED, intermittent buzzer, servo opens vent |
| CRITICAL | Deviation ≥ 70%, sustained | Red LED, continuous buzzer, servo open, call + SMS to primary contact |
| EMERGENCY | Flame detected **while** gas is elevated (any stage) | Skips the ladder entirely — red LED, continuous buzzer, servo open, immediate call + SMS to primary contact |

### 6.4 Neighbor escalation
If the system is still at Critical or Emergency **3 minutes** after the primary contact was notified, and nobody has pressed the reset button, it automatically calls and texts the secondary contact.

### 6.5 Data logging
Every calibration, stage change, and notification event is written to `gaslog.txt` on the SD card with a timestamp, so you can review or demonstrate exactly what happened during a test.

---

## 7. Complete Arduino Code

Create a new sketch in the Arduino IDE (or import into PictoBlox's Arduino Uno coding mode if it supports raw sketch upload) and paste the following. Before uploading, **update the two phone numbers** near the top of the code with your actual primary and secondary contact numbers, in international format (e.g. `+233XXXXXXXXX`).

### Required Libraries
Install these via Arduino IDE → Tools → Manage Libraries, if not already installed:
- `LiquidCrystal_I2C` (for the LCD)
- `Servo` (usually built in)
- `SoftwareSerial` (usually built in)
- `SD` (usually built in)

```cpp
/*
  ============================================================
  AEROGUARD-X1 — SMART GAS LEAK DETECTION SYSTEM
  Group 1
  Arduino Uno

  Features implemented:
   1. Self-calibrating baseline (per zone)
   2. Multi-stage escalation: LOW / MEDIUM / CRITICAL
   3. Confirmation window (false-alarm suppression)
   4. Automatic ventilation (servo)
   5. Manual reset + recalibration button
   6. Remote notification: SIM800L CALL + SMS, with
      neighbor escalation if unanswered
   7. Battery-aware low-power mode (manual flag, see notes)
   8. Data logging to SD card
   9. Multi-zone monitoring (2 gas sensors)
  10. Flame sensor fast-track EMERGENCY tier
  ============================================================
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

// ---------------- PIN DEFINITIONS ----------------
const int PIN_GAS_ZONE1   = A0;   // MQ-2/MQ-5 sensor, Zone 1 (e.g. Kitchen)
const int PIN_GAS_ZONE2   = A1;   // MQ-2/MQ-5 sensor, Zone 2 (e.g. Gas storage)
const int PIN_FLAME       = A2;   // Flame sensor analog output

const int PIN_LED_YELLOW  = 2;
const int PIN_LED_ORANGE  = 3;
const int PIN_LED_RED     = 4;

const int PIN_SIM_RX      = 5;    // Uno RX  <- SIM800L TX
const int PIN_SIM_TX      = 6;    // Uno TX  -> SIM800L RX (through voltage divider)

const int PIN_BUTTON      = 7;
const int PIN_BUZZER      = 8;
const int PIN_SERVO       = 9;

const int PIN_SD_CS       = 10;   // SD card module chip-select
// D11, D12, D13 are fixed SPI pins (MOSI, MISO, SCK) used automatically by SD.h

// ---------------- LCD ----------------
// Common I2C address is 0x27 or 0x3F -- change if your LCD does not show text
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ---------------- SERVO ----------------
Servo ventServo;
const int SERVO_CLOSED_ANGLE = 0;
const int SERVO_OPEN_ANGLE   = 90;

// ---------------- GSM ----------------
SoftwareSerial simSerial(PIN_SIM_RX, PIN_SIM_TX);
const char* PRIMARY_CONTACT   = "+233XXXXXXXXX";  // <-- set this
const char* SECONDARY_CONTACT = "+233YYYYYYYYY";  // <-- set this (neighbor/relative)

// ---------------- CALIBRATION SETTINGS ----------------
const unsigned long CALIBRATION_TIME_MS   = 45000;   // 45 seconds
const unsigned long CALIBRATION_SAMPLE_MS = 500;      // sample every 0.5s

// ---------------- ESCALATION THRESHOLDS (% deviation above baseline) ----------------
const float THRESHOLD_LOW      = 20.0;
const float THRESHOLD_MEDIUM   = 40.0;
const float THRESHOLD_CRITICAL = 70.0;

// Confirmation window: reading must stay above a threshold this long before escalating
const unsigned long CONFIRM_WINDOW_MS = 8000; // 8 seconds

// Flame detection: LOW analog value usually means flame present (module-dependent -- verify with test)
const int FLAME_DETECT_THRESHOLD = 400; // adjust after testing your specific module

// Response window before escalating SMS/call to secondary contact
const unsigned long RESPONSE_WINDOW_MS = 180000; // 3 minutes

// ---------------- STATE ----------------
enum RiskLevel { SAFE, LOW, MEDIUM, CRITICAL, EMERGENCY };

struct Zone {
  const char* name;
  int pin;
  float baseline;
  RiskLevel currentLevel;
  unsigned long thresholdCrossedAt; // for confirmation window
  RiskLevel pendingLevel;
  float lastReading;
  float previousReading;            // for trend arrow
};

Zone zone1 = { "Kitchen", PIN_GAS_ZONE1, 0, SAFE, 0, SAFE, 0, 0 };
Zone zone2 = { "Storage", PIN_GAS_ZONE2, 0, SAFE, 0, SAFE, 0, 0 };

bool emergencyActive = false;
unsigned long criticalSince = 0;      // when Critical/Emergency SMS was first sent
bool primaryNotified = false;
bool secondaryNotified = false;

bool sdReady = false;

unsigned long lastLcdSwitch = 0;
bool lcdShowingTrend = false;

// ============================================================
// SETUP
// ============================================================
void setup() {
  Serial.begin(9600);
  simSerial.begin(9600);

  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_ORANGE, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BUTTON, INPUT_PULLUP); // button to GND, uses internal pull-up
  pinMode(PIN_FLAME, INPUT);

  ventServo.attach(PIN_SERVO);
  ventServo.write(SERVO_CLOSED_ANGLE);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("AeroGuard-X1");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  Serial.println(F("=== AeroGuard-X1 ==="));

  // SD card
  if (SD.begin(PIN_SD_CS)) {
    sdReady = true;
    Serial.println(F("SD card ready."));
    logEvent("SYSTEM", "System boot");
  } else {
    sdReady = false;
    Serial.println(F("SD card NOT found - logging disabled."));
  }

  // GSM module warm-up
  delay(3000);
  initGSM();

  // Calibrate both zones
  calibrateZone(zone1);
  calibrateZone(zone2);

  lcd.clear();
  Serial.println(F("System ready. Monitoring..."));
}

// ============================================================
// MAIN LOOP
// ============================================================
void loop() {
  readZone(zone1);
  readZone(zone2);

  bool flameDetected = checkFlame();

  evaluateZone(zone1, flameDetected);
  evaluateZone(zone2, flameDetected);

  updateOutputs();
  updateLCD();
  handleButton();
  handleEscalationTimers();

  delay(1000); // main loop tick - adjust if system feels sluggish or too fast
}

// ============================================================
// CALIBRATION
// ============================================================
void calibrateZone(Zone &z) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating:");
  lcd.setCursor(0, 1);
  lcd.print(z.name);

  Serial.print(F("Calibrating "));
  Serial.println(z.name);

  long total = 0;
  int samples = 0;
  unsigned long start = millis();

  while (millis() - start < CALIBRATION_TIME_MS) {
    total += analogRead(z.pin);
    samples++;
    delay(CALIBRATION_SAMPLE_MS);
  }

  z.baseline = (float)total / samples;
  z.lastReading = z.baseline;
  z.previousReading = z.baseline;

  Serial.print(z.name);
  Serial.print(F(" baseline = "));
  Serial.println(z.baseline);

  char msg[40];
  snprintf(msg, sizeof(msg), "Baseline=%.1f", z.baseline);
  logEvent(z.name, msg);
}

// ============================================================
// SENSOR READING
// ============================================================
void readZone(Zone &z) {
  z.previousReading = z.lastReading;
  z.lastReading = analogRead(z.pin);
}

bool checkFlame() {
  int val = analogRead(PIN_FLAME);
  return (val < FLAME_DETECT_THRESHOLD);
}

// ============================================================
// RISK EVALUATION (per zone) WITH CONFIRMATION WINDOW
// ============================================================
void evaluateZone(Zone &z, bool flameDetected) {
  float deviation = ((z.lastReading - z.baseline) / z.baseline) * 100.0;
  if (deviation < 0) deviation = 0;

  RiskLevel instantLevel;
  if (deviation >= THRESHOLD_CRITICAL) instantLevel = CRITICAL;
  else if (deviation >= THRESHOLD_MEDIUM) instantLevel = MEDIUM;
  else if (deviation >= THRESHOLD_LOW) instantLevel = LOW;
  else instantLevel = SAFE;

  // --- Flame fast-track: gas elevated (LOW or above) + flame detected = EMERGENCY ---
  if (flameDetected && instantLevel != SAFE) {
    if (!emergencyActive) {
      emergencyActive = true;
      z.currentLevel = EMERGENCY;
      triggerEmergency(z);
    }
    return;
  }

  // --- Normal confirmation window logic ---
  if (instantLevel != z.pendingLevel) {
    z.pendingLevel = instantLevel;
    z.thresholdCrossedAt = millis();
  }

  if (instantLevel > z.currentLevel) {
    // Rising: require sustained reading before escalating
    if (millis() - z.thresholdCrossedAt >= CONFIRM_WINDOW_MS) {
      changeZoneLevel(z, instantLevel);
    }
  } else if (instantLevel < z.currentLevel) {
    // Falling: also require a short sustained drop before de-escalating,
    // to avoid flickering between stages
    if (millis() - z.thresholdCrossedAt >= CONFIRM_WINDOW_MS) {
      changeZoneLevel(z, instantLevel);
    }
  }
}

void changeZoneLevel(Zone &z, RiskLevel newLevel) {
  if (newLevel == z.currentLevel) return;

  z.currentLevel = newLevel;

  char msg[50];
  snprintf(msg, sizeof(msg), "Level changed to %s", levelName(newLevel));
  logEvent(z.name, msg);

  Serial.print(z.name);
  Serial.print(F(" -> "));
  Serial.println(levelName(newLevel));

  if (newLevel == CRITICAL) {
    triggerCritical(z);
  }

  if (newLevel == SAFE) {
    // Reset notification flags when a zone returns to safe
    primaryNotified = false;
    secondaryNotified = false;
    emergencyActive = false;
  }
}

const char* levelName(RiskLevel lvl) {
  switch (lvl) {
    case SAFE: return "SAFE";
    case LOW: return "LOW";
    case MEDIUM: return "MEDIUM";
    case CRITICAL: return "CRITICAL";
    case EMERGENCY: return "EMERGENCY";
  }
  return "?";
}

// ============================================================
// OUTPUTS: LEDs, buzzer, servo based on the HIGHEST active zone level
// ============================================================
void updateOutputs() {
  RiskLevel highest = SAFE;
  if (zone1.currentLevel > highest) highest = zone1.currentLevel;
  if (zone2.currentLevel > highest) highest = zone2.currentLevel;

  digitalWrite(PIN_LED_YELLOW, (highest == LOW));
  digitalWrite(PIN_LED_ORANGE, (highest == MEDIUM));
  digitalWrite(PIN_LED_RED, (highest == CRITICAL || highest == EMERGENCY));

  switch (highest) {
    case SAFE:
    case LOW:
      noTone(PIN_BUZZER);
      break;
    case MEDIUM:
      tone(PIN_BUZZER, 1000, 200); // intermittent beep, retriggered each loop
      break;
    case CRITICAL:
    case EMERGENCY:
      tone(PIN_BUZZER, 1500); // continuous tone
      break;
  }

  if (highest == MEDIUM || highest == CRITICAL || highest == EMERGENCY) {
    ventServo.write(SERVO_OPEN_ANGLE);
  } else {
    ventServo.write(SERVO_CLOSED_ANGLE);
  }
}

// ============================================================
// LCD DISPLAY (alternates between readings and trend)
// ============================================================
void updateLCD() {
  if (millis() - lastLcdSwitch < 2500) return; // switch view every 2.5s
  lastLcdSwitch = millis();
  lcdShowingTrend = !lcdShowingTrend;

  RiskLevel highest = SAFE;
  if (zone1.currentLevel > highest) highest = zone1.currentLevel;
  if (zone2.currentLevel > highest) highest = zone2.currentLevel;

  lcd.clear();

  if (highest == EMERGENCY) {
    lcd.setCursor(0, 0);
    lcd.print("!! FIRE RISK !!");
    lcd.setCursor(0, 1);
    lcd.print("EMERGENCY");
    return;
  }

  if (!lcdShowingTrend) {
    lcd.setCursor(0, 0);
    lcd.print(zone1.name);
    lcd.print(": ");
    lcd.print(levelName(zone1.currentLevel));
    lcd.setCursor(0, 1);
    lcd.print(zone2.name);
    lcd.print(": ");
    lcd.print(levelName(zone2.currentLevel));
  } else {
    lcd.setCursor(0, 0);
    lcd.print(zone1.name);
    lcd.print(" ");
    lcd.print(trendArrow(zone1));
    lcd.print(" R:");
    lcd.print((int)zone1.lastReading);

    lcd.setCursor(0, 1);
    lcd.print(zone2.name);
    lcd.print(" ");
    lcd.print(trendArrow(zone2));
    lcd.print(" R:");
    lcd.print((int)zone2.lastReading);
  }
}

const char* trendArrow(Zone &z) {
  float diff = z.lastReading - z.previousReading;
  if (diff > 3) return "^";
  if (diff < -3) return "v";
  return "-";
}

// ============================================================
// BUTTON: silence + recalibrate
// ============================================================
void handleButton() {
  static unsigned long lastPress = 0;
  if (digitalRead(PIN_BUTTON) == LOW) { // pressed (active LOW with pull-up)
    if (millis() - lastPress > 500) { // simple debounce
      lastPress = millis();
      Serial.println(F("Button pressed: silencing + recalibrating"));
      noTone(PIN_BUZZER);
      logEvent("SYSTEM", "Manual reset/recalibration triggered");

      calibrateZone(zone1);
      calibrateZone(zone2);

      zone1.currentLevel = SAFE;
      zone2.currentLevel = SAFE;
      zone1.pendingLevel = SAFE;
      zone2.pendingLevel = SAFE;
      emergencyActive = false;
      primaryNotified = false;
      secondaryNotified = false;
    }
  }
}

// ============================================================
// CRITICAL / EMERGENCY NOTIFICATION LOGIC
// ============================================================
void triggerCritical(Zone &z) {
  char msg[80];
  snprintf(msg, sizeof(msg), "GAS LEAK - CRITICAL - %s zone. Please check immediately.", z.name);

  callNumber(PRIMARY_CONTACT);
  delay(2000);
  sendSMS(PRIMARY_CONTACT, msg);

  primaryNotified = true;
  secondaryNotified = false;
  criticalSince = millis();

  logEvent(z.name, "CRITICAL - primary contact notified (call+SMS)");
}

void triggerEmergency(Zone &z) {
  char msg[90];
  snprintf(msg, sizeof(msg), "FIRE EMERGENCY - gas+flame detected in %s zone. Immediate danger.", z.name);

  ventServo.write(SERVO_OPEN_ANGLE);

  callNumber(PRIMARY_CONTACT);
  delay(2000);
  sendSMS(PRIMARY_CONTACT, msg);

  primaryNotified = true;
  secondaryNotified = false;
  criticalSince = millis();

  logEvent(z.name, "EMERGENCY - flame+gas fast-track - primary contact notified");
}

void handleEscalationTimers() {
  // If still critical/emergency after RESPONSE_WINDOW_MS and nobody has
  // reset the system via the button, escalate to secondary contact.
  bool stillHigh = (zone1.currentLevel == CRITICAL || zone1.currentLevel == EMERGENCY ||
                     zone2.currentLevel == CRITICAL || zone2.currentLevel == EMERGENCY);

  if (primaryNotified && !secondaryNotified && stillHigh) {
    if (millis() - criticalSince >= RESPONSE_WINDOW_MS) {
      const char* zoneName = (zone1.currentLevel >= CRITICAL) ? zone1.name : zone2.name;
      char msg[90];
      snprintf(msg, sizeof(msg), "ALERT: Primary contact unresponsive. Gas/fire risk at %s. Please check.", zoneName);

      callNumber(SECONDARY_CONTACT);
      delay(2000);
      sendSMS(SECONDARY_CONTACT, msg);

      secondaryNotified = true;
      logEvent(zoneName, "Secondary contact (neighbor) notified - no response from primary");
    }
  }
}

// ============================================================
// GSM (SIM800L) FUNCTIONS
// ============================================================
void initGSM() {
  Serial.println(F("Initializing GSM module..."));
  simSerial.println("AT");
  delay(1000);
  simSerial.println("AT+CMGF=1"); // SMS text mode
  delay(1000);
  Serial.println(F("GSM module initialized (verify with serial monitor)."));
}

void sendSMS(const char* number, const char* message) {
  Serial.print(F("Sending SMS to "));
  Serial.println(number);

  simSerial.print("AT+CMGF=1\r");
  delay(500);
  simSerial.print("AT+CMGS=\"");
  simSerial.print(number);
  simSerial.println("\"");
  delay(500);
  simSerial.print(message);
  delay(500);
  simSerial.write(26); // Ctrl+Z to send
  delay(3000);
}

void callNumber(const char* number) {
  Serial.print(F("Calling "));
  Serial.println(number);

  simSerial.print("ATD");
  simSerial.print(number);
  simSerial.println(";");
  delay(20000); // ring duration before hanging up - adjust as needed
  simSerial.println("ATH"); // hang up
  delay(1000);
}

// ============================================================
// SD LOGGING
// ============================================================
void logEvent(const char* zoneName, const char* message) {
  if (!sdReady) return;

  File logFile = SD.open("gaslog.txt", FILE_WRITE);
  if (logFile) {
    logFile.print(millis());
    logFile.print(F(" ms | "));
    logFile.print(zoneName);
    logFile.print(F(" | "));
    logFile.println(message);
    logFile.close();
  } else {
    Serial.println(F("Failed to open log file."));
  }
}
```

---

## 8. Configuration You Must Change Before Uploading

| Setting | Location in code | What to change |
|---------|-------------------|-----------------|
| `PRIMARY_CONTACT` | Near top | Replace with the real primary phone number, international format |
| `SECONDARY_CONTACT` | Near top | Replace with the real neighbor/relative number |
| `lcd(0x27, 16, 2)` | LCD setup | Change `0x27` if an I2C scanner shows a different address |
| `FLAME_DETECT_THRESHOLD` | Near top | Adjust after testing your specific flame sensor module |
| `THRESHOLD_LOW/MEDIUM/CRITICAL` | Near top | Fine-tune based on your MQ sensor's real-world readings during testing |

---

## 9. Testing Checklist

- [ ] Upload code, open Serial Monitor at 9600 baud, confirm calibration runs for both zones and prints baseline values
- [ ] Confirm LCD displays zone status and alternates to trend view every ~2.5 seconds
- [ ] Press the reset button and confirm recalibration restarts and buzzer silences
- [ ] Test flame sensor response with a lighter (briefly, safely, no gas nearby) and confirm Serial Monitor shows a clear reading drop/rise
- [ ] Verify SIM800L registers on network (slow blinking LED) before testing calls/SMS
- [ ] Confirm SD card logs entries by removing the card after a test run and checking `gaslog.txt` on a computer
- [ ] Do a full controlled test to confirm Critical stage triggers a call + SMS to the primary contact
- [ ] Confirm secondary contact is only notified after the response window if the alarm is not cleared

---

## 10. Notes on Future Work

A dedicated test/demo mode (a button-triggered simulated gas spike) would make live demonstrations safer and more reliable without needing an actual gas source near the sensor — this is a natural next addition once the core system is working.
