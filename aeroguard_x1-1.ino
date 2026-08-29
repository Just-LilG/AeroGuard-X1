/*
  AEROGUARD-X1 — Arduino Uno firmware with ESP32 WiFi bridge on A1/A3
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

const int PIN_GAS = A0;
const int PIN_APP_RX = A1;  // Uno RX <- ESP32 TX
const int PIN_FLAME = A2;
const int PIN_APP_TX = A3;  // Uno TX -> ESP32 RX
const int PIN_LED_GREEN = 2;
const int PIN_LED_YELLOW = 3;
const int PIN_LED_RED = 4;
const int PIN_SIM_RX = 5;
const int PIN_SIM_TX = 6;
const int PIN_BTN_RESET = 7;
const int PIN_BUZZER = 8;
const int PIN_BTN_DEMO = 9;
const int PIN_SD_CS = 10;

LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial simSerial(PIN_SIM_RX, PIN_SIM_TX);
SoftwareSerial appSerial(PIN_APP_RX, PIN_APP_TX);  // ESP32 WiFi bridge
const char* OWNER_CONTACT = "+233XXXXXXXXX";
const char* SECONDARY_CONTACT = "+233YYYYYYYYY";
const char* DEVICE_LABEL = "AeroGuard Kitchen";

const unsigned long CALIBRATION_TIME_MS = 45000;
const unsigned long CALIBRATION_SAMPLE_MS = 500;
const float THRESHOLD_LOW = 20.0;
const float THRESHOLD_MEDIUM = 40.0;
const float THRESHOLD_CRITICAL = 70.0;
const unsigned long CONFIRM_WINDOW_MS = 8000;
const unsigned long RESPONSE_WINDOW_MS = 180000;
const int FLAME_DETECT_THRESHOLD = 400;
const unsigned long BTN_DEBOUNCE_MS = 400;

enum RiskLevel { SAFE = 0, LOW = 1, MEDIUM = 2, CRITICAL = 3, FIRE = 4 };
float gasBaseline = 0;
float gasReading = 0;
float gasPrevious = 0;
RiskLevel currentLevel = SAFE;
RiskLevel pendingLevel = SAFE;
unsigned long thresholdCrossedAt = 0;
bool demoMode = false;
int demoStage = 0;
bool ownerNotified = false;
bool secondaryNotified = false;
unsigned long criticalSince = 0;
bool sdReady = false;
unsigned long lastLcdSwitch = 0;
bool lcdAltView = false;

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
  if (SD.begin(PIN_SD_CS)) { sdReady = true; logEvent("SYSTEM", "boot"); }
  simSerial.begin(9600);
  delay(2000);
  initGSM();
  appSerial.begin(9600);  // ESP32 bridge
  calibrateGas();
  setLevel(SAFE, true);
  lcd.clear();
  Serial.println(F("Ready. Demo btn = simulate leak."));
}

void loop() {
  handleDemoButton();
  handleResetButton();
  pollAppBridge();
  if (!demoMode) readSensorsAndEvaluate();
  updateOutputs();
  updateLCD();
  handleSecondarySmsTimer();
  emitAppStatus();
  delay(200);
}

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
  logEvent("DEMO", levelName(staged));
  setLevel(staged, false);
}

void handleResetButton() {
  static unsigned long last = 0;
  if (digitalRead(PIN_BTN_RESET) != LOW) return;
  if (millis() - last < BTN_DEBOUNCE_MS) return;
  last = millis();
  noTone(PIN_BUZZER);
  demoMode = false;
  demoStage = 0;
  ownerNotified = false;
  secondaryNotified = false;
  logEvent("SYSTEM", "reset");
  calibrateGas();
  setLevel(SAFE, true);
}

void calibrateGas() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating...");
  long total = 0;
  int samples = 0;
  unsigned long start = millis();
  while (millis() - start < CALIBRATION_TIME_MS) {
    total += analogRead(PIN_GAS);
    samples++;
    delay(CALIBRATION_SAMPLE_MS);
  }
  gasBaseline = (float)total / samples;
  if (gasBaseline < 1) gasBaseline = 1;
  gasReading = gasBaseline;
  gasPrevious = gasBaseline;
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
  if (flame && instant != SAFE) { setLevel(FIRE, false); return; }
  if (instant != pendingLevel) { pendingLevel = instant; thresholdCrossedAt = millis(); }
  if (instant != currentLevel && millis() - thresholdCrossedAt >= CONFIRM_WINDOW_MS)
    setLevel(instant, false);
}

void setLevel(RiskLevel level, bool silent) {
  if (level == currentLevel && !silent) return;
  RiskLevel previous = currentLevel;
  currentLevel = level;
  pendingLevel = level;
  if (!silent) {
    if (level == MEDIUM && previous < MEDIUM) notifyMedium();
    else if (level == CRITICAL && previous < CRITICAL) notifyCritical();
    else if (level == FIRE && previous != FIRE) notifyFire();
  }
  if (level == SAFE) { ownerNotified = false; secondaryNotified = false; }
}

void notifyMedium() {
  char msg[120];
  snprintf(msg, sizeof(msg), "AeroGuard MEDIUM at %s", DEVICE_LABEL);
  sendSMS(OWNER_CONTACT, msg);
  appPrintln(F("APP_CMD:VENT_OPEN"));
  logEvent("ALERT", "MEDIUM");
}

void notifyCritical() {
  char msg[120];
  snprintf(msg, sizeof(msg), "AeroGuard CRITICAL at %s", DEVICE_LABEL);
  callNumber(OWNER_CONTACT);
  delay(1500);
  sendSMS(OWNER_CONTACT, msg);
  appPrintln(F("APP_CMD:VENT_OPEN"));
  ownerNotified = true; secondaryNotified = false; criticalSince = millis();
  logEvent("ALERT", "CRITICAL");
}

void notifyFire() {
  char msg[120];
  snprintf(msg, sizeof(msg), "AeroGuard FIRE at %s", DEVICE_LABEL);
  callNumber(OWNER_CONTACT);
  delay(1500);
  sendSMS(OWNER_CONTACT, msg);
  appPrintln(F("APP_CMD:VENT_OPEN"));
  ownerNotified = true; secondaryNotified = false; criticalSince = millis();
  logEvent("ALERT", "FIRE");
}

void handleSecondarySmsTimer() {
  bool stillHigh = (currentLevel == CRITICAL || currentLevel == FIRE);
  if (!ownerNotified || secondaryNotified || !stillHigh) return;
  if (millis() - criticalSince < RESPONSE_WINDOW_MS) return;
  char msg[120];
  snprintf(msg, sizeof(msg), "AeroGuard backup: %s still %s", DEVICE_LABEL, levelName(currentLevel));
  sendSMS(SECONDARY_CONTACT, msg);
  secondaryNotified = true;
}

void updateOutputs() {
  digitalWrite(PIN_LED_GREEN, currentLevel == LOW);
  digitalWrite(PIN_LED_YELLOW, currentLevel == MEDIUM);
  digitalWrite(PIN_LED_RED, currentLevel == CRITICAL || currentLevel == FIRE);
  switch (currentLevel) {
    case SAFE: case LOW: noTone(PIN_BUZZER); break;
    case MEDIUM: tone(PIN_BUZZER, 1000, 180); break;
    case CRITICAL: case FIRE: tone(PIN_BUZZER, 1600); break;
  }
}

void updateLCD() {
  if (millis() - lastLcdSwitch < 2000) return;
  lastLcdSwitch = millis();
  lcdAltView = !lcdAltView;
  lcd.clear();
  if (currentLevel == FIRE) {
    lcd.setCursor(0, 0); lcd.print("FIRE RISK");
    lcd.setCursor(0, 1); lcd.print(demoMode ? "DEMO MODE" : "EVACUATE");
    return;
  }
  if (!lcdAltView) {
    lcd.setCursor(0, 0); lcd.print(demoMode ? "DEMO " : "LIVE "); lcd.print(levelName(currentLevel));
    lcd.setCursor(0, 1); lcd.print(DEVICE_LABEL);
  } else {
    lcd.setCursor(0, 0); lcd.print("Gas:"); lcd.print((int)gasReading);
    lcd.setCursor(0, 1); lcd.print("Base:"); lcd.print((int)gasBaseline);
  }
}

const char* levelName(RiskLevel lvl) {
  switch (lvl) {
    case SAFE: return "SAFE"; case LOW: return "LOW"; case MEDIUM: return "MEDIUM";
    case CRITICAL: return "CRITICAL"; case FIRE: return "FIRE";
  }
  return "?";
}

void appPrint(const __FlashStringHelper* s) { Serial.print(s); appSerial.print(s); }
void appPrint(const char* s) { Serial.print(s); appSerial.print(s); }
void appPrint(int n) { Serial.print(n); appSerial.print(n); }
void appPrintln(const __FlashStringHelper* s) { Serial.println(s); appSerial.println(s); }
void appPrintln() { Serial.println(); appSerial.println(); }

void emitAppStatus() {
  static unsigned long last = 0;
  if (millis() - last < 1000) return;
  last = millis();
  appPrint(F("STATUS level="));
  appPrint(levelName(currentLevel));
  appPrint(F(" demo="));
  appPrint(demoMode ? 1 : 0);
  appPrint(F(" gas="));
  appPrint((int)gasReading);
  appPrint(F(" vent=APP"));
  appPrintln();
}

void pollAppBridge() {
  static char buf[48];
  static byte idx = 0;
  while (appSerial.available()) {
    char c = (char)appSerial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      buf[idx] = 0; idx = 0;
      if (strncmp(buf, "APP_CMD:", 8) == 0) {
        Serial.print(F("ESP32 cmd: ")); Serial.println(buf); logEvent("APP", buf);
      }
      continue;
    }
    if (idx < sizeof(buf) - 1) buf[idx++] = c;
  }
}

void initGSM() {
  simSerial.listen();
  simSerial.println("AT"); delay(800);
  simSerial.println("AT+CMGF=1"); delay(800);
  appSerial.listen();
}

void sendSMS(const char* number, const char* message) {
  simSerial.listen();
  simSerial.print("AT+CMGF=1"); simSerial.write('\r'); delay(400);
  simSerial.print("AT+CMGS="); simSerial.write('"');
  simSerial.print(number); simSerial.write('"'); simSerial.println(); delay(400);
  simSerial.print(message); delay(400); simSerial.write(26); delay(2500);
  appSerial.listen();
}

void callNumber(const char* number) {
  simSerial.listen();
  simSerial.print("ATD"); simSerial.print(number); simSerial.println(";");
  delay(18000); simSerial.println("ATH"); delay(800);
  appSerial.listen();
}

void logEvent(const char* tag, const char* message) {
  if (!sdReady) return;
  File f = SD.open("gaslog.txt", FILE_WRITE);
  if (!f) return;
  f.print(millis()); f.print(F(" | ")); f.print(tag); f.print(F(" | ")); f.println(message);
  f.close();
}
