/*
  ============================================================
  AEROGUARD-X1
  Smart Gas Leak Detection System
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
