/*
  AEROGUARD-X1 — Arduino Uno firmware (this is the demo brain).
  LCD is the 4-wire board: GND, VCC, SDA, SCL only.
  SDA → Uno A4. SCL → Uno A5. VCC → 5V. GND → GND.
  SIM800L: 3.7V cell on VCC. Never Uno 5V on SIM VCC.
  Leave D10–D13 empty (no SD). Leave A1 and A3 empty.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#include <string.h>

const int PIN_GAS = A0;
const int PIN_APP_RX = A1;  // leave empty
const int PIN_FLAME = A2;
const int PIN_APP_TX = A3;  // leave empty
const int PIN_LED_GREEN = 2;
const int PIN_LED_YELLOW = 3;
const int PIN_LED_RED = 4;
const int PIN_SIM_RX = 5;
const int PIN_SIM_TX = 6;
const int PIN_BTN_RESET = 7;
const int PIN_BUZZER = 8;
const int PIN_BTN_DEMO = 9;
const int PIN_SD_CS = 10;

const byte LCD_COLS = 16;
const byte LCD_ROWS = 2;
LiquidCrystal_I2C lcd27(0x27, LCD_COLS, LCD_ROWS);
LiquidCrystal_I2C lcd3f(0x3F, LCD_COLS, LCD_ROWS);
LiquidCrystal_I2C *lcdPtr = &lcd27;
#define lcd (*lcdPtr)
SoftwareSerial simSerial(PIN_SIM_RX, PIN_SIM_TX);
SoftwareSerial appSerial(PIN_APP_RX, PIN_APP_TX);  // unused in the demo kit
const char* OWNER_CONTACT = "+233557164067";
const char* SECONDARY_CONTACT = "+233508705321";
const char* DEVICE_LABEL = "AeroGuard Kitchen";
// 3.7V cell is on SIM VCC. Set false only if you unplug that cell.
const bool GSM_ENABLED = true;
// Set true only after the micro SD module is wired to D10–D13.
const bool SD_ENABLED = false;

const unsigned long CALIBRATION_TIME_MS = 1200;
const unsigned long CALIBRATION_QUICK_MS = 400;
const unsigned long CALIBRATION_SAMPLE_MS = 40;
const float THRESHOLD_LOW = 20.0;
const float THRESHOLD_MEDIUM = 40.0;
const float THRESHOLD_CRITICAL = 70.0;
const unsigned long CONFIRM_WINDOW_MS = 8000;
const unsigned long RESPONSE_WINDOW_MS = 180000;
const int FLAME_DETECT_THRESHOLD = 400;
const unsigned long BTN_DEBOUNCE_MS = 80;
const unsigned long GSM_RING_MS = 18000;

// Arduino already uses the word LOW for "pin off". Do not name a stage LOW.
enum RiskLevel { SAFE = 0, LEVEL_LOW = 1, MEDIUM = 2, CRITICAL = 3, FIRE = 4 };
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
unsigned long lastLcdPage = 0;
bool lcdAltView = false;
bool lcdTick = false;

enum GsmState {
  GSM_IDLE,
  GSM_DIAL,
  GSM_RINGING,
  GSM_HANG,
  GSM_SMS_MODE,
  GSM_SMS_ADDR,
  GSM_SMS_BODY,
  GSM_SMS_WAIT
};
GsmState gsmState = GSM_IDLE;
char gsmNum[20];
char gsmText[96];
unsigned long gsmStepAt = 0;
bool gsmSmsAfterCall = false;

void lcdLine(byte row, const char* text) {
  if (row >= LCD_ROWS) return;
  lcd.setCursor(0, row);
  for (byte i = 0; i < LCD_COLS; i++) {
    char c = text[i];
    if (c == 0) {
      while (i < LCD_COLS) { lcd.print(' '); i++; }
      return;
    }
    lcd.print(c);
  }
}

void lcdClearAll() {
  lcd.clear();
  lcdLine(0, "");
  lcdLine(1, "");
}

void pickLcd() {
  Wire.begin();
  delay(80);
  bool got27 = false;
  bool got3f = false;
  Serial.print(F("I2C scan:"));
  for (byte a = 1; a < 127; a++) {
    Wire.beginTransmission(a);
    if (Wire.endTransmission() == 0) {
      Serial.print(F(" 0x"));
      Serial.print(a, HEX);
      if (a == 0x27) got27 = true;
      if (a == 0x3F) got3f = true;
    }
  }
  Serial.println();
  if (got3f && !got27) lcdPtr = &lcd3f;
  else lcdPtr = &lcd27;
}

void bootSplash() {
  lcdClearAll();
  lcdLine(0, "AeroGuard-X1");
  lcdLine(1, "");
  for (byte k = 0; k < LCD_COLS; k++) {
    lcd.setCursor(k, 1);
    lcd.write(255);
    delay(45);
  }
  lcdLine(1, "Starting...");
  delay(350);
}

void setup() {
  Serial.begin(9600);
  delay(200);
  Serial.println(F("=== AeroGuard-X1 v1 ==="));
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_LED_YELLOW, OUTPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_BTN_RESET, INPUT_PULLUP);
  pinMode(PIN_BTN_DEMO, INPUT_PULLUP);
  pinMode(PIN_FLAME, INPUT);
  pickLcd();
  lcd.init();
  lcd.backlight();
  bootSplash();
  if (SD_ENABLED && SD.begin(PIN_SD_CS)) { sdReady = true; logEvent("SYSTEM", "boot"); }
  else { Serial.println(F("No SD module (OK for this bench).")); }
  appSerial.begin(9600);
  calibrateGas(false);
  setLevel(SAFE, true);
  Serial.println(F("Ready. Demo: touch D9 to GND."));
}

bool gsmStarted = false;

void loop() {
  handleDemoButton();
  handleResetButton();
  pumpGsm();
  pollAppBridge();
  if (!demoMode) readSensorsAndEvaluate();
  updateOutputs();
  updateLCD();
  handleSecondarySmsTimer();
  emitAppStatus();
  delay(15);
}

void handleDemoButton() {
  static unsigned long last = 0;
  if (digitalRead(PIN_BTN_DEMO) != LOW) return;
  if (millis() - last < BTN_DEBOUNCE_MS) return;
  last = millis();
  Serial.println(F("DEMO pressed"));
  demoMode = true;
  demoStage++;
  if (demoStage > 4) demoStage = 1;
  RiskLevel staged = SAFE;
  if (demoStage == 1) staged = LEVEL_LOW;
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
  cancelGsm();
  noTone(PIN_BUZZER);
  demoMode = false;
  demoStage = 0;
  ownerNotified = false;
  secondaryNotified = false;
  currentLevel = SAFE;
  pendingLevel = SAFE;
  updateOutputs();
  logEvent("SYSTEM", "reset");
  calibrateGas(true);
  setLevel(SAFE, true);
}

void calibrateGas(bool quick) {
  (void)quick;
  gasBaseline = (float)analogRead(PIN_GAS);
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
  else if (deviation >= THRESHOLD_LOW) instant = LEVEL_LOW;
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
  if (level == SAFE) { ownerNotified = false; secondaryNotified = false; }
  lastLcdSwitch = 0;
  lastLcdPage = 0;
  lcdAltView = false;
  updateOutputs();
  updateLCD();
  if (!silent) {
    if (level == MEDIUM && previous < MEDIUM) notifyMedium();
    else if (level == CRITICAL && previous < CRITICAL) notifyCritical();
    else if (level == FIRE && previous != FIRE) notifyFire();
  }
}

void notifyMedium() {
  char msg[96];
  snprintf(msg, sizeof(msg), "AeroGuard MEDIUM at %s", DEVICE_LABEL);
  queueSms(OWNER_CONTACT, msg);
  appPrintln(F("APP_CMD:VENT_OPEN"));
  logEvent("ALERT", "MEDIUM");
}

void notifyCritical() {
  char msg[96];
  snprintf(msg, sizeof(msg), "AeroGuard CRITICAL at %s", DEVICE_LABEL);
  queueCallThenSms(OWNER_CONTACT, msg);
  appPrintln(F("APP_CMD:VENT_OPEN"));
  ownerNotified = true; secondaryNotified = false; criticalSince = millis();
  logEvent("ALERT", "CRITICAL");
}

void notifyFire() {
  char msg[96];
  snprintf(msg, sizeof(msg), "AeroGuard FIRE at %s", DEVICE_LABEL);
  queueCallThenSms(OWNER_CONTACT, msg);
  appPrintln(F("APP_CMD:VENT_OPEN"));
  ownerNotified = true; secondaryNotified = false; criticalSince = millis();
  logEvent("ALERT", "FIRE");
}

void handleSecondarySmsTimer() {
  bool stillHigh = (currentLevel == CRITICAL || currentLevel == FIRE);
  if (!ownerNotified || secondaryNotified || !stillHigh) return;
  if (millis() - criticalSince < RESPONSE_WINDOW_MS) return;
  if (gsmState != GSM_IDLE) return;
  char msg[96];
  snprintf(msg, sizeof(msg), "AeroGuard backup: %s still %s", DEVICE_LABEL, levelName(currentLevel));
  queueSms(SECONDARY_CONTACT, msg);
  secondaryNotified = true;
}

void updateOutputs() {
  if (currentLevel == SAFE) {
    digitalWrite(PIN_LED_GREEN, (millis() / 500) % 2);
    digitalWrite(PIN_LED_YELLOW, LOW);
    digitalWrite(PIN_LED_RED, LOW);
  } else {
    digitalWrite(PIN_LED_GREEN, currentLevel == LEVEL_LOW);
    digitalWrite(PIN_LED_YELLOW, currentLevel == MEDIUM);
    digitalWrite(PIN_LED_RED, currentLevel == CRITICAL || currentLevel == FIRE);
  }
  switch (currentLevel) {
    case SAFE: case LEVEL_LOW: noTone(PIN_BUZZER); break;
    case MEDIUM: tone(PIN_BUZZER, 1000, 180); break;
    case CRITICAL: case FIRE: tone(PIN_BUZZER, 1600); break;
  }
}

void updateLCD() {
  unsigned long now = millis();
  if (lastLcdSwitch != 0 && now - lastLcdSwitch < 400) return;
  lastLcdSwitch = now;
  lcdTick = !lcdTick;
  char star = lcdTick ? '*' : ' ';

  if (lastLcdPage == 0 || now - lastLcdPage >= 2000) {
    if (lastLcdPage != 0) lcdAltView = !lcdAltView;
    lastLcdPage = now;
  }

  if (currentLevel == SAFE) {
    lcdLine(0, "READY      SAFE");
    lcdLine(1, "Touch D9 to GND");
    return;
  }

  if (currentLevel == FIRE) {
    char fire0[17];
    snprintf(fire0, 17, "FIRE RISK     %c", star);
    lcdLine(0, fire0);
    lcdLine(1, demoMode ? "DEMO MODE" : "EVACUATE NOW");
    return;
  }

  char line0[17];
  char line1[17];
  if (!lcdAltView) {
    snprintf(line0, 17, "%s %-7s%c", demoMode ? "DEMO" : "LIVE", levelName(currentLevel), star);
    snprintf(line1, 17, "%.16s", DEVICE_LABEL);
  } else {
    snprintf(line0, 17, "Gas %4d     %c", (int)gasReading, star);
    snprintf(line1, 17, "Base %4d", (int)gasBaseline);
  }
  lcdLine(0, line0);
  lcdLine(1, line1);
}

const char* levelName(RiskLevel lvl) {
  switch (lvl) {
    case SAFE: return "SAFE"; case LEVEL_LOW: return "LOW"; case MEDIUM: return "MEDIUM";
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
        Serial.print(F("APP cmd: ")); Serial.println(buf); logEvent("APP", buf);
      }
      continue;
    }
    if (idx < sizeof(buf) - 1) buf[idx++] = c;
  }
}

void initGSM() {
  if (!GSM_ENABLED) return;
  if (gsmStarted) return;
  gsmStarted = true;
  simSerial.begin(9600);
  simSerial.listen();
  simSerial.println("AT");
  simSerial.println("AT+CMGF=1");
  appSerial.listen();
}

void copyGsm(const char* number, const char* message) {
  strncpy(gsmNum, number, sizeof(gsmNum) - 1);
  gsmNum[sizeof(gsmNum) - 1] = 0;
  strncpy(gsmText, message, sizeof(gsmText) - 1);
  gsmText[sizeof(gsmText) - 1] = 0;
}

void cancelGsm() {
  if (gsmState == GSM_RINGING || gsmState == GSM_DIAL) {
    simSerial.listen();
    simSerial.println("ATH");
  }
  gsmState = GSM_IDLE;
  gsmSmsAfterCall = false;
}

void queueSms(const char* number, const char* message) {
  if (!GSM_ENABLED) return;
  initGSM();
  cancelGsm();
  copyGsm(number, message);
  gsmSmsAfterCall = false;
  gsmState = GSM_SMS_MODE;
}

void queueCallThenSms(const char* number, const char* message) {
  if (!GSM_ENABLED) return;
  initGSM();
  cancelGsm();
  copyGsm(number, message);
  gsmSmsAfterCall = true;
  gsmState = GSM_DIAL;
}

void pumpGsm() {
  if (!GSM_ENABLED || gsmState == GSM_IDLE) return;
  unsigned long now = millis();
  switch (gsmState) {
    case GSM_DIAL:
      simSerial.listen();
      simSerial.print(F("ATD"));
      simSerial.print(gsmNum);
      simSerial.println(F(";"));
      gsmStepAt = now;
      gsmState = GSM_RINGING;
      break;
    case GSM_RINGING:
      if (now - gsmStepAt < GSM_RING_MS) return;
      simSerial.println(F("ATH"));
      gsmStepAt = now;
      gsmState = GSM_HANG;
      break;
    case GSM_HANG:
      if (now - gsmStepAt < 500) return;
      if (gsmSmsAfterCall) {
        gsmSmsAfterCall = false;
        gsmState = GSM_SMS_MODE;
      } else {
        gsmState = GSM_IDLE;
        appSerial.listen();
      }
      break;
    case GSM_SMS_MODE:
      simSerial.listen();
      simSerial.print(F("AT+CMGF=1"));
      simSerial.write('\r');
      gsmStepAt = now;
      gsmState = GSM_SMS_ADDR;
      break;
    case GSM_SMS_ADDR:
      if (now - gsmStepAt < 400) return;
      simSerial.print(F("AT+CMGS="));
      simSerial.write('"');
      simSerial.print(gsmNum);
      simSerial.write('"');
      simSerial.println();
      gsmStepAt = now;
      gsmState = GSM_SMS_BODY;
      break;
    case GSM_SMS_BODY:
      if (now - gsmStepAt < 400) return;
      simSerial.print(gsmText);
      simSerial.write(26);
      gsmStepAt = now;
      gsmState = GSM_SMS_WAIT;
      break;
    case GSM_SMS_WAIT:
      if (now - gsmStepAt < 2500) return;
      gsmState = GSM_IDLE;
      appSerial.listen();
      break;
    default:
      gsmState = GSM_IDLE;
      break;
  }
}

void logEvent(const char* tag, const char* message) {
  if (!sdReady) return;
  File f = SD.open("gaslog.txt", FILE_WRITE);
  if (!f) return;
  f.print(millis()); f.print(F(" | ")); f.print(tag); f.print(F(" | ")); f.println(message);
  f.close();
}
