/*
  AEROGUARD-X1 — Arduino Uno firmware with ESP32 WiFi bridge on A1/A3
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <string.h>

const int PIN_GAS = A0;
const int PIN_APP_RX = A1;  // Uno RX <- ESP32 TX
const int PIN_FLAME = A2;
const int PIN_APP_TX = A3;  // Uno TX -> ESP32 RX
const int PIN_LED_GREEN = 2;
const int PIN_LED_YELLOW = 3;
const int PIN_LED_RED = 4;
const int PIN_SIM_RX = 5;  // SIM800L TX → Uno (leave unwired until 4V buck)
const int PIN_SIM_TX = 6;  // Uno → SIM800L RX via 10k/20k (leave until buck)
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

const unsigned long CALIBRATION_TIME_MS = 6000;
const unsigned long CALIBRATION_SAMPLE_MS = 80;
const float THRESHOLD_LOW = 20.0;
const float THRESHOLD_MEDIUM = 40.0;
const float THRESHOLD_CRITICAL = 70.0;
const unsigned long CONFIRM_WINDOW_MS = 8000;
const unsigned long RESPONSE_WINDOW_MS = 180000;
const int FLAME_DETECT_THRESHOLD = 400;
const unsigned long BTN_DEBOUNCE_MS = 60;

enum RiskLevel { SAFE = 0, STAGE_LOW = 1, MEDIUM = 2, CRITICAL = 3, FIRE = 4 };
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
bool gsmReady = false;
unsigned long lastLcdSwitch = 0;
unsigned long lastAnimMs = 0;
bool lcdAltView = false;
bool lcdBlink = false;

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
  {
    byte fill[8] = {0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
    lcd.createChar(0, fill);
  }
  bootSplash();
  Serial.println(F("=== AeroGuard-X1 v1 ==="));
  if (SD.begin(PIN_SD_CS)) { sdReady = true; logEvent("SYSTEM", "boot"); }
  simSerial.begin(9600);
  delay(200);
  initGSM();
  appSerial.begin(9600);
  calibrateGas();
  setLevel(SAFE, true);
  paintLcd(true);
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
  delay(40);
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
  if (demoStage == 1) staged = STAGE_LOW;
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

void bootSplash() {
  lcd.clear();
  const char title[] = "AeroGuard-X1";
  for (byte i = 0; i < 12; i++) {
    lcd.setCursor(i, 0);
    lcd.print(title[i]);
    digitalWrite(PIN_LED_GREEN, i % 3 == 0);
    digitalWrite(PIN_LED_YELLOW, i % 3 == 1);
    digitalWrite(PIN_LED_RED, i % 3 == 2);
    delay(45);
  }
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  lcd.setCursor(0, 1);
  lcd.print("LPG + fire warn");
  delay(350);
}

void calibrateGas() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrate");
  long total = 0;
  int samples = 0;
  unsigned long start = millis();
  const char spin[] = {'|', '/', '-', '\\'};
  byte frame = 0;
  while (millis() - start < CALIBRATION_TIME_MS) {
    total += analogRead(PIN_GAS);
    samples++;
    unsigned long el = millis() - start;
    if (el > CALIBRATION_TIME_MS) el = CALIBRATION_TIME_MS;
    int filled = (int)((el * 10UL) / CALIBRATION_TIME_MS);
    int pct = (int)((el * 100UL) / CALIBRATION_TIME_MS);
    if (filled > 10) filled = 10;
    if (pct > 99) pct = 99;

    lcd.setCursor(10, 0);
    lcd.print(spin[frame & 3]);
    lcd.print(' ');
    lcd.setCursor(0, 1);
    lcd.print('[');
    for (int i = 0; i < 10; i++) {
      if (i < filled) lcd.write((uint8_t)0);
      else lcd.print(' ');
    }
    lcd.print(']');
    lcd.print(pct);
    if (pct < 10) lcd.print(' ');
    lcd.print('%');

    digitalWrite(PIN_LED_GREEN, (frame % 3) == 0);
    digitalWrite(PIN_LED_YELLOW, (frame % 3) == 1);
    digitalWrite(PIN_LED_RED, (frame % 3) == 2);
    frame++;
    delay(CALIBRATION_SAMPLE_MS);
  }
  digitalWrite(PIN_LED_GREEN, LOW);
  digitalWrite(PIN_LED_YELLOW, LOW);
  digitalWrite(PIN_LED_RED, LOW);
  if (samples < 1) samples = 1;
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
  else if (deviation >= THRESHOLD_LOW) instant = STAGE_LOW;
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
  updateOutputs();
  paintLcd(true);
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
  sendSMS(OWNER_CONTACT, msg);
  appPrintln(F("APP_CMD:VENT_OPEN"));
  ownerNotified = true; secondaryNotified = false; criticalSince = millis();
  logEvent("ALERT", "CRITICAL");
}

void notifyFire() {
  char msg[120];
  snprintf(msg, sizeof(msg), "AeroGuard FIRE at %s", DEVICE_LABEL);
  callNumber(OWNER_CONTACT);
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
  digitalWrite(PIN_LED_GREEN, currentLevel == STAGE_LOW);
  digitalWrite(PIN_LED_YELLOW, currentLevel == MEDIUM);
  digitalWrite(PIN_LED_RED, currentLevel == CRITICAL || currentLevel == FIRE);
  switch (currentLevel) {
    case SAFE: case STAGE_LOW: noTone(PIN_BUZZER); break;
    case MEDIUM: tone(PIN_BUZZER, 1000, 180); break;
    case CRITICAL: case FIRE: tone(PIN_BUZZER, 1600); break;
  }
}

void lcdLine(uint8_t row, const char* a, const char* b) {
  char buf[17];
  memset(buf, ' ', 16);
  buf[16] = 0;
  size_t n = 0;
  if (a) {
    while (a[n] && n < 16) { buf[n] = a[n]; n++; }
  }
  if (b) {
    size_t i = 0;
    while (b[i] && n < 16) { buf[n++] = b[i++]; }
  }
  lcd.setCursor(0, row);
  lcd.print(buf);
}

void paintLcd(bool force) {
  if (!force) {
    if (millis() - lastLcdSwitch < 2000) return;
    lastLcdSwitch = millis();
    lcdAltView = !lcdAltView;
  } else {
    lastLcdSwitch = millis();
  }

  if (currentLevel == FIRE) {
    if (lcdBlink) {
      lcdLine(0, demoMode ? "DEMO  FIRE" : "FIRE RISK", 0);
      lcdLine(1, demoMode ? "Reset to exit" : "EVACUATE NOW", 0);
    } else {
      lcdLine(0, "  !!  FIRE  !! ", 0);
      lcdLine(1, "              ", 0);
    }
    return;
  }

  if (currentLevel == CRITICAL && lcdBlink && !lcdAltView) {
    lcdLine(0, demoMode ? "DEMO CRITICAL" : "CRITICAL", 0);
    lcdLine(1, "GET OUT / CALL", 0);
    return;
  }

  if (!lcdAltView || force) {
    char top[17];
    snprintf(top, sizeof(top), "%s%c %-7s",
             demoMode ? "DEMO" : "LIVE",
             (demoMode && lcdBlink) ? '*' : ' ',
             levelName(currentLevel));
    lcdLine(0, top, 0);
    lcdLine(1, DEVICE_LABEL, 0);
  } else {
    char g[17], b[17];
    snprintf(g, sizeof(g), "Gas %d", (int)gasReading);
    snprintf(b, sizeof(b), "Base %d", (int)gasBaseline);
    lcdLine(0, g, 0);
    lcdLine(1, b, 0);
  }
}

void updateLCD() {
  bool hot = (currentLevel == CRITICAL || currentLevel == FIRE);
  unsigned long now = millis();
  if (hot || demoMode) {
    if (now - lastAnimMs < 280) return;
    lastAnimMs = now;
    lcdBlink = !lcdBlink;
    paintLcd(true);
    return;
  }
  paintLcd(false);
}

const char* levelName(RiskLevel lvl) {
  switch (lvl) {
    case SAFE: return "SAFE"; case STAGE_LOW: return "LOW"; case MEDIUM: return "MEDIUM";
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

bool gsmGotOk(unsigned long waitMs) {
  unsigned long start = millis();
  char line[24];
  byte n = 0;
  while (millis() - start < waitMs) {
    while (simSerial.available()) {
      char c = (char)simSerial.read();
      if (c == '\r') continue;
      if (c == '\n') {
        line[n] = 0;
        n = 0;
        if (strstr(line, "OK")) return true;
        continue;
      }
      if (n < sizeof(line) - 1) line[n++] = c;
    }
  }
  return false;
}

void initGSM() {
  gsmReady = false;
  simSerial.listen();
  while (simSerial.available()) simSerial.read();
  simSerial.println("AT");
  gsmReady = gsmGotOk(300);
  if (gsmReady) {
    simSerial.println("AT+CMGF=1");
    gsmGotOk(300);
  } else {
    Serial.println(F("GSM skip (no AT) — Demo will not wait on call/SMS"));
  }
  appSerial.listen();
}

void sendSMS(const char* number, const char* message) {
  if (!gsmReady) return;
  simSerial.listen();
  simSerial.print("AT+CMGF=1"); simSerial.write('\r'); delay(400);
  simSerial.print("AT+CMGS="); simSerial.write('"');
  simSerial.print(number); simSerial.write('"'); simSerial.println(); delay(400);
  simSerial.print(message); delay(400); simSerial.write(26); delay(2500);
  appSerial.listen();
}

void callNumber(const char* number) {
  if (!gsmReady) return;
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
