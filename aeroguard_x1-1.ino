/*
  ============================================================
  AEROGUARD-X1
  Smart LPG Leak + Fire Early-Warning Device
  Competition / product firmware — Arduino Uno

  Product behaviour (v1):
   - 1 gas zone (expandable later) + flame sensor
   - Stages: SAFE → LOW (green) → MEDIUM (yellow) → CRITICAL (red)
   - FIRE: flame while gas elevated (or demo stage 4)
   - Demo button: simulate leak for presentations (no real gas)
   - Reset button: silence + exit demo + recalibrate
   - GSM: SMS on MEDIUM+; CALL+SMS on CRITICAL/FIRE to owner
   - Secondary contact: SMS only if owner does not clear in time
   - Vent/window actuation: app-side (smart vents) — not on-device servo
   - SD: local incident history (app is primary log later)
   - BLE pins reserved for AeroGuard app link
  ============================================================
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

// ---------------- PIN MAP ----------------
const int PIN_GAS         = A0;   // MQ-2 / MQ-5 analog out
const int PIN_FLAME       = A2;   // Flame sensor analog out
// A1, A3 reserved for HM-10 BLE (app link) — see build guide
const int PIN_LED_GREEN   = 2;    // LOW
const int PIN_LED_YELLOW  = 3;    // MEDIUM
const int PIN_LED_RED     = 4;    // CRITICAL / FIRE
const int PIN_SIM_RX      = 5;    // Uno RX  <- SIM800L TX
const int PIN_SIM_TX      = 6;    // Uno TX  -> SIM800L RX (via voltage divider)
const int PIN_BTN_RESET   = 7;    // Mute + exit demo + recalibrate (to GND)
const int PIN_BUZZER      = 8;
const int PIN_BTN_DEMO    = 9;    // Demo stage advance (to GND)
const int PIN_SD_CS       = 10;   // D11/D12/D13 = SPI

// ---------------- LCD ----------------
LiquidCrystal_I2C lcd(0x27, 16, 2);  // change to 0x3F if needed

// ---------------- GSM ----------------
SoftwareSerial simSerial(PIN_SIM_RX, PIN_SIM_TX);
const char* OWNER_CONTACT     = "+233XXXXXXXXX";  // <-- set before demo
const char* SECONDARY_CONTACT = "+233YYYYYYYYY";  // optional backup SMS (roommate/landlord)
const char* DEVICE_LABEL      = "AeroGuard Kitchen";  // shown in SMS text

// ---------------- TIMING / THRESHOLDS ----------------
const unsigned long CALIBRATION_TIME_MS   = 45000;
const unsigned long CALIBRATION_SAMPLE_MS = 500;
const float THRESHOLD_LOW      = 20.0;   // % above baseline
const float THRESHOLD_MEDIUM   = 40.0;
const float THRESHOLD_CRITICAL = 70.0;
const unsigned long CONFIRM_WINDOW_MS  = 8000;
const unsigned long RESPONSE_WINDOW_MS = 180000;  // 3 min → secondary SMS
const int FLAME_DETECT_THRESHOLD = 400;           // calibrate your module
const unsigned long BTN_DEBOUNCE_MS = 400;

// ---------------- STATE ----------------
enum RiskLevel { SAFE = 0, LOW = 1, MEDIUM = 2, CRITICAL = 3, FIRE = 4 };

float gasBaseline = 0;
float gasReading = 0;
float gasPrevious = 0;
RiskLevel currentLevel = SAFE;
RiskLevel pendingLevel = SAFE;
unsigned long thresholdCrossedAt = 0;

bool demoMode = false;
int demoStage = 0;  // 0=SAFE live, 1=LOW, 2=MEDIUM, 3=CRITICAL, 4=FIRE

bool ownerNotified = false;
bool secondaryNotified = false;
unsigned long criticalSince = 0;
bool sdReady = false;

unsigned long lastLcdSwitch = 0;
bool lcdAltView = false;

// ============================================================
void setup() {
  Serial.begin(9600);

  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);
  pinMode(PIN_BTN_DEMO, INPUT_PULLUP);
  pinMode(PIN_FLAME, INPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("AeroGuard-X1");
  lcd.setCursor(0, 1);
  lcd.print("Starting...");

  Serial.println(F("=== AeroGuard-X1 v1 ==="));

  if (SD.begin(PIN_SD_CS)) {
    sdReady = true;
    Serial.println(F("SD ready"));
    logEvent("SYSTEM", "boot");
  } else {
    Serial.println(F("SD not found — logging off"));
  }

  simSerial.begin(9600);
  delay(2000);
  initGSM();

  calibrateGas();
  setLevel(SAFE, true);

  lcd.clear();
  Serial.println(F("Ready. Demo btn = simulate leak. Reset = clear."));
}

void loop() {
  handleDemoButton();
  handleResetButton();

  if (!demoMode) {
    readSensorsAndEvaluate();
  }

  updateOutputs();
  updateLCD();
  handleSecondarySmsTimer();
  emitAppStatus();  // Serial (and later BLE) status line for the app

  delay(200);
}

// ============================================================
// DEMO: staged gas-leak simulation for presentation
// Press 1 → LOW, 2 → MEDIUM, 3 → CRITICAL, 4 → FIRE
// ============================================================
void handleDemoButton() {
  static unsigned long last = 0;
  if (digitalRead(PIN_BTN_DEMO) != LOW) return;
  if (millis() - last < BTN_DEBOUNCE_MS) return;
  last = millis();

  demoMode = true;
  demoStage++;
  if (demoStage > 4) demoStage = 1;

  RiskLevel staged = SAFE;
  if (demoStage == 1) staged = LOW;
  else if (demoStage == 2) staged = MEDIUM;
  else if (demoStage == 3) staged = CRITICAL;
  else if (demoStage == 4) staged = FIRE;

  Serial.print(F("DEMO stage "));
  Serial.print(demoStage);
  Serial.print(F(" -> "));
  Serial.println(levelName(staged));
  logEvent("DEMO", levelName(staged));
  setLevel(staged, false);
}

void handleResetButton() {
  static unsigned long last = 0;
  if (digitalRead(PIN_BTN_RESET) != LOW) return;
  if (millis() - last < BTN_DEBOUNCE_MS) return;
  last = millis();

  Serial.println(F("RESET: mute, exit demo, recalibrate"));
  noTone(PIN_BUZZER);
  demoMode = false;
  demoStage = 0;
  ownerNotified = false;
  secondaryNotified = false;
  logEvent("SYSTEM", "reset");

  calibrateGas();
  setLevel(SAFE, true);
}

// ============================================================
// LIVE SENSING
// ============================================================
void calibrateGas() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating...");
  lcd.setCursor(0, 1);
  lcd.print("Keep air clear");

  long total = 0;
  int samples = 0;
  unsigned long start = millis();
  while (millis() - start < CALIBRATION_TIME_MS) {
    total += analogRead(PIN_GAS);
    samples++;
    delay(CALIBRATION_SAMPLE_MS);
  }
  gasBaseline = (float)total / samples;
  gasReading = gasBaseline;
  gasPrevious = gasBaseline;

  char msg[40];
  snprintf(msg, sizeof(msg), "baseline=%.1f", gasBaseline);
  logEvent("GAS", msg);
  Serial.println(msg);
}

void readSensorsAndEvaluate() {
  gasPrevious = gasReading;
  gasReading = analogRead(PIN_GAS);

  float deviation = ((gasReading - gasBaseline) / gasBaseline) * 100.0;
  if (deviation < 0) deviation = 0;

  RiskLevel instant = SAFE;
  if (deviation >= THRESHOLD_CRITICAL) instant = CRITICAL;
  else if (deviation >= THRESHOLD_MEDIUM) instant = MEDIUM;
  else if (deviation >= THRESHOLD_LOW) instant = LOW;

  bool flame = (analogRead(PIN_FLAME) < FLAME_DETECT_THRESHOLD);
  if (flame && instant != SAFE) {
    setLevel(FIRE, false);
    return;
  }

  if (instant != pendingLevel) {
    pendingLevel = instant;
    thresholdCrossedAt = millis();
  }

  if (instant != currentLevel) {
    if (millis() - thresholdCrossedAt >= CONFIRM_WINDOW_MS) {
      setLevel(instant, false);
    }
  }
}

void setLevel(RiskLevel level, bool silent) {
  if (level == currentLevel && !silent) return;

  RiskLevel previous = currentLevel;
  currentLevel = level;
  pendingLevel = level;

  if (!silent) {
    char msg[48];
    snprintf(msg, sizeof(msg), "level %s", levelName(level));
    logEvent("STATE", msg);
  }

  // Notifications only on rising edges into MEDIUM / CRITICAL / FIRE
  if (!silent) {
    if (level == MEDIUM && previous < MEDIUM) {
      notifyMedium();
    } else if (level == CRITICAL && previous < CRITICAL) {
      notifyCritical();
    } else if (level == FIRE && previous != FIRE) {
      notifyFire();
    }
  }

  if (level == SAFE) {
    ownerNotified = false;
    secondaryNotified = false;
  }
}

// ============================================================
// ALERTS
// ============================================================
void notifyMedium() {
  char msg[120];
  snprintf(msg, sizeof(msg),
           "AeroGuard MEDIUM: possible LPG rise at %s. Check stove/cylinder. Open windows if safe.",
           DEVICE_LABEL);
  sendSMS(OWNER_CONTACT, msg);
  // App will command linked smart vents/windows — device only signals intent:
  Serial.println(F("APP_CMD:VENT_OPEN"));
  logEvent("ALERT", "MEDIUM SMS owner + vent intent");
}

void notifyCritical() {
  char msg[120];
  snprintf(msg, sizeof(msg),
           "AeroGuard CRITICAL: LPG leak risk at %s. Leave area if strong smell. Do not spark.",
           DEVICE_LABEL);
  callNumber(OWNER_CONTACT);
  delay(1500);
  sendSMS(OWNER_CONTACT, msg);
  Serial.println(F("APP_CMD:VENT_OPEN"));
  ownerNotified = true;
  secondaryNotified = false;
  criticalSince = millis();
  logEvent("ALERT", "CRITICAL call+SMS owner");
}

void notifyFire() {
  char msg[120];
  snprintf(msg, sizeof(msg),
           "AeroGuard FIRE: flame+gas at %s. Evacuate. Call fire service if needed.",
           DEVICE_LABEL);
  callNumber(OWNER_CONTACT);
  delay(1500);
  sendSMS(OWNER_CONTACT, msg);
  Serial.println(F("APP_CMD:VENT_OPEN"));
  // Fire-service dispatch = Phase 2 (needs verified address partner) — not a silent voice call
  ownerNotified = true;
  secondaryNotified = false;
  criticalSince = millis();
  logEvent("ALERT", "FIRE call+SMS owner");
}

void handleSecondarySmsTimer() {
  bool stillHigh = (currentLevel == CRITICAL || currentLevel == FIRE);
  if (!ownerNotified || secondaryNotified || !stillHigh) return;
  if (millis() - criticalSince < RESPONSE_WINDOW_MS) return;

  char msg[120];
  snprintf(msg, sizeof(msg),
           "AeroGuard backup: %s still in %s after 3 min. Please check on occupants.",
           DEVICE_LABEL, levelName(currentLevel));
  sendSMS(SECONDARY_CONTACT, msg);  // SMS only — not expected to enter a gas scene
  secondaryNotified = true;
  logEvent("ALERT", "secondary SMS");
}

// ============================================================
// OUTPUTS (no on-device servo — vents are app/smart-home)
// ============================================================
void updateOutputs() {
  digitalWrite(PIN_LED_GREEN,  currentLevel == LOW);
  digitalWrite(PIN_LED_YELLOW, currentLevel == MEDIUM);
  digitalWrite(PIN_LED_RED,    currentLevel == CRITICAL || currentLevel == FIRE);

  // SAFE: LEDs off (or brief green heartbeat later). LOW: green solid, quiet.
  switch (currentLevel) {
    case SAFE:
      noTone(PIN_BUZZER);
      break;
    case LOW:
      noTone(PIN_BUZZER);
      break;
    case MEDIUM:
      tone(PIN_BUZZER, 1000, 180);
      break;
    case CRITICAL:
    case FIRE:
      tone(PIN_BUZZER, 1600);
      break;
  }
}

void updateLCD() {
  if (millis() - lastLcdSwitch < 2000) return;
  lastLcdSwitch = millis();
  lcdAltView = !lcdAltView;
  lcd.clear();

  if (currentLevel == FIRE) {
    lcd.setCursor(0, 0);
    lcd.print("FIRE RISK");
    lcd.setCursor(0, 1);
    lcd.print(demoMode ? "DEMO MODE" : "EVACUATE");
    return;
  }

  if (!lcdAltView) {
    lcd.setCursor(0, 0);
    lcd.print(demoMode ? "DEMO " : "LIVE ");
    lcd.print(levelName(currentLevel));
    lcd.setCursor(0, 1);
    lcd.print(DEVICE_LABEL);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Gas:");
    lcd.print((int)gasReading);
    lcd.print(trendArrow());
    lcd.setCursor(0, 1);
    lcd.print("Base:");
    lcd.print((int)gasBaseline);
  }
}

const char* trendArrow() {
  float d = gasReading - gasPrevious;
  if (d > 3) return " ^";
  if (d < -3) return " v";
  return " -";
}

const char* levelName(RiskLevel lvl) {
  switch (lvl) {
    case SAFE: return "SAFE";
    case LOW: return "LOW";
    case MEDIUM: return "MEDIUM";
    case CRITICAL: return "CRITICAL";
    case FIRE: return "FIRE";
  }
  return "?";
}

// Status line for Serial Monitor / future BLE app bridge
void emitAppStatus() {
  static unsigned long last = 0;
  if (millis() - last < 1000) return;
  last = millis();
  Serial.print(F("STATUS level="));
  Serial.print(levelName(currentLevel));
  Serial.print(F(" demo="));
  Serial.print(demoMode ? 1 : 0);
  Serial.print(F(" gas="));
  Serial.print((int)gasReading);
  Serial.print(F(" vent=APP"));
  Serial.println();
}

// ============================================================
// GSM
// ============================================================
void initGSM() {
  simSerial.println("AT");
  delay(800);
  simSerial.println("AT+CMGF=1");
  delay(800);
  Serial.println(F("GSM init sent"));
}

void sendSMS(const char* number, const char* message) {
  Serial.print(F("SMS -> "));
  Serial.println(number);
  simSerial.print("AT+CMGF=1\r");
  delay(400);
  simSerial.print("AT+CMGS=\"");
  simSerial.print(number);
  simSerial.println("\"");
  delay(400);
  simSerial.print(message);
  delay(400);
  simSerial.write(26);
  delay(2500);
}

void callNumber(const char* number) {
  Serial.print(F("CALL -> "));
  Serial.println(number);
  simSerial.print("ATD");
  simSerial.print(number);
  simSerial.println(";");
  delay(18000);
  simSerial.println("ATH");
  delay(800);
}

// ============================================================
// SD — local incident history (not a fire-proof black box)
// ============================================================
void logEvent(const char* tag, const char* message) {
  if (!sdReady) return;
  File f = SD.open("gaslog.txt", FILE_WRITE);
  if (!f) return;
  f.print(millis());
  f.print(F(" | "));
  f.print(tag);
  f.print(F(" | "));
  f.println(message);
  f.close();
}
