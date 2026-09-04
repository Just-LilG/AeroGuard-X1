/*
  AeroGuard-X1 — written from scratch. Same pin map.

  What this box does:
  - Green = safe / low. Yellow = medium. Red = critical / fire.
  - Demo button (D9 joined to GND) is only a fire drill.
  - Boot show, then a short calibrating show, then READY.
  - Buzzer keeps buzzing while a text or call goes out.
  - Real gas on A0, or flame on A2, raises the alarm with no Demo press.
  - Reset button (D7 joined to GND) goes back to safe.
  - The phone chip (SIM800L) only wakes when we send a warning. Not at boot.

  Screen: 4 wires. GND, VCC, SDA, SCL.
    GND → GND     VCC → 5V     SDA → A4     SCL → A5

  Pins (do not change):
    A0  gas analog
    A1  empty
    A2  flame analog
    A3  empty
    A4  LCD SDA
    A5  LCD SCL
    D2  green LED
    D3  yellow LED
    D4  red LED
    D5  SIM TXD (straight wire)
    D6  SIM RXD (through 10k resistors)
    D7  Reset → GND
    D8  buzzer
    D9  Demo → GND
    D10–D13 empty

  SIM VCC from a 3.7V cell only. Never Uno 5V.
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <string.h>

// ---- same pin map ----
const int PIN_GAS = A0;
const int PIN_FLAME = A2;
const int PIN_GREEN = 2;
const int PIN_YELLOW = 3;
const int PIN_RED = 4;
const int PIN_SIM_RX = 5;   // Uno listens. Wire to SIM TXD.
const int PIN_SIM_TX = 6;   // Uno talks. Wire to SIM RXD through 10k.
const int PIN_RESET = 7;
const int PIN_BUZZER = 8;
const int PIN_DEMO = 9;

// 16 letters × 2 rows. If the screen is blank, try 0x3F instead of 0x27.
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial sim(PIN_SIM_RX, PIN_SIM_TX);

const char* OWNER_CONTACT = "+233557164067";
const char* SECONDARY_CONTACT = "+233599494342";
const char* DEVICE_LABEL = "AeroGuard Kitchen";

// true = send real texts/calls. Set false if the 3.7V cell is unplugged.
const bool PHONE_ALERTS = true;
// SMS wait. Keep texts short.
const unsigned long SMS_BURST_MS = 3000;
// A real call needs longer than 3s. The other phone has not even started ringing yet at 3s.
const unsigned long CALL_RING_MS = 15000;
// Show "calling now" / "texting" for this long, then hide. Call and SMS keep going.
const unsigned long PHONE_UI_MS = 3000;
const unsigned long BOOT_MS = 2200;
const unsigned long CAL_MS = 3200;

// Flame analog drops when it sees fire light. Lower this if a lighter never trips.
const int FLAME_DETECT_THRESHOLD = 400;
// How far the gas number must climb above quiet air.
const int GAS_LOW_ABOVE = 40;
const int GAS_MED_ABOVE = 90;
const int GAS_CRIT_ABOVE = 160;

// 0=safe  1=low  2=medium  3=critical  4=fire
int level = 0;
bool demoMode = false;

int gasCalm = 0;
int lastDemo = HIGH;
int lastReset = HIGH;

unsigned long lcdAt = 0;
unsigned long beepAt = 0;
unsigned long warnAt = 0;
int lastWarned = -1;

int liveHold = 0;
unsigned long liveHoldAt = 0;
unsigned long liveReadyAt = 0;
int lastGas = 0;
int lastFlame = 0;
unsigned long logAt = 0;
int showPhase = 0;  // 0 boot, 1 calibrating, 2 running
unsigned long showAt = 0;
bool buzzOn = false;

// Phone work is a short list of steps. One step, wait, next step.
int phoneStep = 0;
unsigned long phoneAt = 0;
char phoneTo[20];
char phoneText[80];
char phoneUi[17];
unsigned long phoneUiAt = 0;
bool phoneAlsoSms = false;
bool phoneIsBackup = false;
bool backupDone = false;
int phoneTries = 0;

char simBuf[48];
byte simBufLen = 0;
bool simOK = false;
bool simErr = false;
bool simPrompt = false;

void padLine(const char* text) {
  int n = 0;
  while (text[n] && n < 16) {
    lcd.print(text[n]);
    n++;
  }
  while (n < 16) {
    lcd.print(' ');
    n++;
  }
}

void paint(const char* top, const char* bottom) {
  lcd.setCursor(0, 0);
  padLine(top);
  lcd.setCursor(0, 1);
  padLine(bottom);
}

const char* levelWord() {
  if (level == 0) return "SAFE";
  if (level == 1) return "LOW";
  if (level == 2) return "MEDIUM";
  if (level == 3) return "CRITICAL";
  return "FIRE";
}

void silenceBuzzer() {
  digitalWrite(PIN_BUZZER, LOW);
  noTone(PIN_BUZZER);
  buzzOn = false;
}

void setLights() {
  digitalWrite(PIN_GREEN, LOW);
  digitalWrite(PIN_YELLOW, LOW);
  digitalWrite(PIN_RED, LOW);

  if (showPhase == 0) {
    unsigned long t = millis() - showAt;
    digitalWrite(PIN_GREEN, t > 200);
    digitalWrite(PIN_YELLOW, t > 700);
    digitalWrite(PIN_RED, t > 1200);
    return;
  }

  if (showPhase == 1) {
    digitalWrite(PIN_GREEN, (millis() / 250) % 2);
    return;
  }

  if (level == 0) {
    digitalWrite(PIN_GREEN, (millis() / 500) % 2);
  } else if (level == 1) {
    digitalWrite(PIN_GREEN, HIGH);
  } else if (level == 2) {
    digitalWrite(PIN_YELLOW, HIGH);
  } else {
    digitalWrite(PIN_RED, HIGH);
    digitalWrite(PIN_YELLOW, (millis() / 200) % 2);
  }
}

void setSound() {
  if (showPhase < 2 || level <= 1) {
    silenceBuzzer();
    return;
  }

  unsigned long wait = 400;
  if (level == 3) wait = 160;
  if (level == 4) wait = 80;

  if (millis() - beepAt >= wait) {
    beepAt = millis();
    buzzOn = !buzzOn;
    digitalWrite(PIN_BUZZER, buzzOn ? HIGH : LOW);
  }
}

void setScreen() {
  if (millis() - lcdAt < 320) return;
  lcdAt = millis();

  char top[17];
  char bottom[17];

  if (showPhase == 0) {
    int frame = (int)(((millis() - showAt) / 400) % 4);
    if (frame == 0) paint("AeroGuard-X1", "waking");
    else if (frame == 1) paint("AeroGuard-X1", "waking.");
    else if (frame == 2) paint("AeroGuard-X1", "waking..");
    else paint("AeroGuard-X1", "waking...");
    return;
  }

  if (showPhase == 1) {
    unsigned long t = millis() - showAt;
    int fill = (int)(t * 14 / CAL_MS);
    if (fill < 1) fill = 1;
    if (fill > 14) fill = 14;
    bottom[0] = '[';
    for (int i = 0; i < 14; i++) bottom[i + 1] = (i < fill) ? '=' : ' ';
    bottom[15] = ']';
    bottom[16] = 0;
    paint("CALIBRATING", bottom);
    return;
  }

  if (level == 0) {
    snprintf(top, 17, "READY      SAFE");
    snprintf(bottom, 17, "gas %4d  D9", lastGas);
    paint(top, bottom);
    return;
  }

  snprintf(top, 17, "ALARM  %-8s", levelWord());
  if (phoneStep != 0 && phoneUi[0]) {
    paint(top, phoneUi);
    return;
  }
  if (demoMode) {
    snprintf(bottom, 17, "DEMO  gas %4d", lastGas);
  } else {
    snprintf(bottom, 17, "LIVE  gas %4d", lastGas);
  }
  paint(top, bottom);
}

void pumpShow() {
  if (showPhase == 0 && millis() - showAt >= BOOT_MS) {
    showPhase = 1;
    showAt = millis();
    lcdAt = 0;
    gasCalm = analogRead(PIN_GAS);
    lastGas = gasCalm;
  }
  if (showPhase == 1 && millis() - showAt >= CAL_MS) {
    showPhase = 2;
    liveReadyAt = millis();
    lcdAt = 0;
  }
}

void skipShow() {
  showPhase = 2;
  liveReadyAt = millis();
  lcdAt = 0;
  digitalWrite(PIN_YELLOW, LOW);
  digitalWrite(PIN_RED, LOW);
}

void setPhoneUi(const char* text) {
  strncpy(phoneUi, text, 16);
  phoneUi[16] = 0;
  phoneUiAt = millis();
  lcdAt = 0;
}

void hidePhoneUiIfDue() {
  if (!phoneUi[0]) return;
  if (millis() - phoneUiAt < PHONE_UI_MS) return;
  phoneUi[0] = 0;
  lcdAt = 0;
}

void simListen() {
  while (sim.available()) {
    char c = (char)sim.read();
    Serial.write(c);
    if (c == '>') simPrompt = true;
    if (c == '\r' || c == '\n') {
      if (simBufLen > 0) {
        simBuf[simBufLen] = 0;
        if (strstr(simBuf, "OK")) simOK = true;
        if (strstr(simBuf, "ERROR")) simErr = true;
        if (strstr(simBuf, "NO CARRIER")) simErr = true;
        if (strstr(simBuf, "NO DIALTONE")) simErr = true;
        if (strstr(simBuf, "BUSY")) simErr = true;
      }
      simBufLen = 0;
    } else if (simBufLen < 46) {
      simBuf[simBufLen++] = c;
    }
  }
}

void simSend(const char* cmd) {
  simOK = false;
  simErr = false;
  simPrompt = false;
  Serial.print(F(">> "));
  Serial.println(cmd);
  sim.println(cmd);
  phoneAt = millis();
}

void hangUp() {
  if (PHONE_ALERTS) sim.println("ATH");
  phoneStep = 0;
  phoneAlsoSms = false;
  phoneIsBackup = false;
  phoneUi[0] = 0;
  if (level <= 1) silenceBuzzer();
}

void startCallThenSms(const char* number, const char* text) {
  if (!PHONE_ALERTS) return;
  strncpy(phoneTo, number, 19);
  phoneTo[19] = 0;
  strncpy(phoneText, text, 79);
  phoneText[79] = 0;
  phoneAlsoSms = true;
  phoneIsBackup = false;
  phoneTries = 0;
  phoneStep = 1;
  phoneAt = millis();
  setPhoneUi("phone waking");
}

void startSmsOnly(const char* number, const char* text) {
  if (!PHONE_ALERTS) return;
  strncpy(phoneTo, number, 19);
  phoneTo[19] = 0;
  strncpy(phoneText, text, 79);
  phoneText[79] = 0;
  phoneAlsoSms = false;
  phoneTries = 0;
  phoneStep = 1;
  phoneAt = millis();
  setPhoneUi(phoneIsBackup ? "text backup" : "phone waking");
}

void phoneDone() {
  phoneStep = 0;
  phoneAlsoSms = false;
  phoneIsBackup = false;
  phoneUi[0] = 0;
  lcdAt = 0;
}

void pumpPhone() {
  if (!PHONE_ALERTS || phoneStep == 0) return;

  hidePhoneUiIfDue();
  simListen();
  unsigned long now = millis();

  // 1: open the talk wire, keep buzzing, say AT
  if (phoneStep == 1) {
    sim.begin(9600);
    delay(20);
    while (sim.available()) sim.read();
    simBufLen = 0;
    simSend("AT");
    phoneTries = 1;
    phoneStep = 2;
    return;
  }

  // 2: keep saying AT until the chip answers OK (auto-baud)
  if (phoneStep == 2) {
    if (simOK) {
      simSend("ATE0");
      phoneStep = 3;
      return;
    }
    if (now - phoneAt > 800) {
      phoneTries++;
      if (phoneTries > 12) {
        Serial.println(F("SIM: no AT. Check cell VCC, GND, D5=TXD, D6=RXD."));
        setPhoneUi("no SIM talk");
        phoneDone();
        return;
      }
      simSend("AT");
    }
    return;
  }

  // 3: echo off
  if (phoneStep == 3) {
    if (simOK || now - phoneAt > 800) {
      simSend("AT+CFUN=1");
      phoneStep = 4;
    }
    return;
  }

  // 4: radio on, then call or text right away (no network wait)
  if (phoneStep == 4) {
    if (simOK || now - phoneAt > 1500) {
      if (phoneAlsoSms) {
        setPhoneUi("calling now");
        sim.print("ATD");
        sim.print(phoneTo);
        sim.println(";");
        Serial.print(F(">> ATD"));
        Serial.print(phoneTo);
        Serial.println(F(";"));
        phoneAt = now;
        phoneStep = 6;
      } else {
        setPhoneUi(phoneIsBackup ? "text backup" : "texting owner");
        simSend("AT+CMGF=1");
        phoneStep = 8;
      }
    }
    return;
  }

  // 6: keep ringing long enough for the other phone to start
  if (phoneStep == 6) {
    if (simErr) {
      Serial.println(F("SIM: call failed"));
      setPhoneUi("call failed");
      simSend("ATH");
      phoneStep = 7;
      return;
    }
    if (now - phoneAt > CALL_RING_MS) {
      simSend("ATH");
      phoneStep = 7;
    }
    return;
  }

  // 7: after hang up, send the text
  if (phoneStep == 7) {
    if (simOK || now - phoneAt > 1200) {
      if (phoneAlsoSms) {
        phoneAlsoSms = false;
        setPhoneUi("texting owner");
        simSend("AT+CMGF=1");
        phoneStep = 8;
      } else {
        phoneDone();
      }
    }
    return;
  }

  // 8: text mode
  if (phoneStep == 8) {
    if (simOK || now - phoneAt > 1200) {
      simOK = false;
      simPrompt = false;
      Serial.print(F(">> AT+CMGS=\""));
      Serial.print(phoneTo);
      Serial.println('"');
      sim.print("AT+CMGS=\"");
      sim.print(phoneTo);
      sim.println('"');
      phoneAt = now;
      phoneStep = 9;
    }
    return;
  }

  // 9: wait for the > prompt, then pour the words and Ctrl+Z
  if (phoneStep == 9) {
    if (simPrompt || now - phoneAt > SMS_BURST_MS) {
      Serial.println(F(">> (sms body)"));
      sim.print(phoneText);
      sim.write(26);
      phoneAt = now;
      phoneStep = 10;
      setPhoneUi("sms sending");
    }
    return;
  }

  // 10: wait for the chip to say the text went out
  if (phoneStep == 10) {
    if (simOK || now - phoneAt > SMS_BURST_MS) {
      Serial.println(simOK ? F("SIM: SMS sent") : F("SIM: SMS timeout"));
      setPhoneUi(simOK ? "sms sent" : "sms timeout");
      phoneDone();
    }
  }
}

void raiseLevel(int next, bool fromDemo) {
  if (next < 0) next = 0;
  if (next > 4) next = 4;
  if (next == level && fromDemo == demoMode) return;

  level = next;
  demoMode = fromDemo;
  lcdAt = 0;
  lastWarned = -1;

  Serial.print(F("Level "));
  Serial.print(levelWord());
  if (fromDemo) Serial.println(F(" (DEMO)"));
  else Serial.println(F(" (LIVE)"));

  if (level == 0) {
    hangUp();
    backupDone = false;
    warnAt = 0;
    silenceBuzzer();
  }
}

void maybeWarnPhones() {
  if (showPhase < 2) return;
  if (!PHONE_ALERTS) return;
  if (level < 2) return;
  if (lastWarned == level) return;
  lastWarned = level;
  warnAt = millis();
  phoneIsBackup = false;

  char text[80];
  if (level == 2) {
    snprintf(text, 80, "AeroGuard MEDIUM gas at %s", DEVICE_LABEL);
    startSmsOnly(OWNER_CONTACT, text);
  } else if (level == 3) {
    snprintf(text, 80, "AeroGuard CRITICAL at %s. Leave now.", DEVICE_LABEL);
    startCallThenSms(OWNER_CONTACT, text);
    backupDone = false;
  } else {
    snprintf(text, 80, "AeroGuard FIRE and gas at %s", DEVICE_LABEL);
    startCallThenSms(OWNER_CONTACT, text);
    backupDone = false;
  }
}

void maybeBackupSms() {
  if (!PHONE_ALERTS) return;
  if (backupDone) return;
  if (level < 2) return;
  if (warnAt == 0) return;
  if (phoneStep != 0) return;

  char text[80];
  snprintf(text, 80, "BACKUP: AeroGuard %s at %s", levelWord(), DEVICE_LABEL);
  phoneIsBackup = true;
  startSmsOnly(SECONDARY_CONTACT, text);
  backupDone = true;
}

void readButtons() {
  int demoNow = digitalRead(PIN_DEMO);
  int resetNow = digitalRead(PIN_RESET);

  // Press = pin just went from HIGH (open) to LOW (joined to GND).
  if (lastDemo == HIGH && demoNow == LOW) {
    if (showPhase < 2) skipShow();
    int next = level + 1;
    if (next > 4) next = 0;
    raiseLevel(next, true);
    delay(40);
  }

  if (lastReset == HIGH && resetNow == LOW) {
    gasCalm = analogRead(PIN_GAS);
    lastGas = gasCalm;
    liveHold = 0;
    liveHoldAt = millis();
    showPhase = 1;
    showAt = millis();
    liveReadyAt = millis() + CAL_MS;
    raiseLevel(0, false);
    delay(40);
  }

  lastDemo = demoNow;
  lastReset = resetNow;
}

int readSmoothGas() {
  long sum = 0;
  for (int i = 0; i < 8; i++) sum += analogRead(PIN_GAS);
  return (int)(sum / 8);
}

void followQuietAir(int gas) {
  // The nose can drift while it warms. Follow slow drift.
  // A leak jumps up, so we do not chase a big climb.
  if (gas < gasCalm) {
    gasCalm = (gasCalm * 7 + gas) / 8;
  } else if ((gas - gasCalm) < 15) {
    gasCalm = (gasCalm * 31 + gas) / 32;
  }
}

int liveLevelFromSensors(int gas, int flame) {
  bool fire = flame < FLAME_DETECT_THRESHOLD;
  int climb = gas - gasCalm;
  if (climb < 0) climb = 0;

  int fromGas = 0;
  if (climb > GAS_CRIT_ABOVE) fromGas = 3;
  else if (climb > GAS_MED_ABOVE) fromGas = 2;
  else if (climb > GAS_LOW_ABOVE) fromGas = 1;

  if (fire && fromGas >= 1) return 4;
  if (fire) return 3;
  return fromGas;
}

void readLiveSensors() {
  lastGas = readSmoothGas();
  lastFlame = analogRead(PIN_FLAME);

  if (showPhase < 2 || millis() < liveReadyAt) {
    gasCalm = lastGas;
    liveHold = 0;
    liveHoldAt = millis();
    return;
  }

  if (level == 0 && !demoMode) followQuietAir(lastGas);

  int next = liveLevelFromSensors(lastGas, lastFlame);
  if (next != liveHold) {
    liveHold = next;
    liveHoldAt = millis();
  }

  unsigned long need = (next >= 3) ? 800 : 1000;
  if (millis() - liveHoldAt < need) return;

  // Demo is only a drill. Real gas or flame can still raise the alarm.
  if (demoMode) {
    if (next > level) raiseLevel(next, false);
    return;
  }

  if (next != level) raiseLevel(next, false);
}

void maybeLogGas() {
  if (millis() - logAt < 1000) return;
  logAt = millis();
  Serial.print(F("gas="));
  Serial.print(lastGas);
  Serial.print(F(" calm="));
  Serial.print(gasCalm);
  Serial.print(F(" flame="));
  Serial.println(lastFlame);
}

void setup() {
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_YELLOW, OUTPUT);
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_DEMO, INPUT_PULLUP);
  pinMode(PIN_RESET, INPUT_PULLUP);

  Serial.begin(9600);
  Serial.println(F("AeroGuard-X1 boot"));
  if (PHONE_ALERTS) sim.begin(9600);

  Wire.begin();
  lcd.init();
  lcd.backlight();
  paint("AeroGuard-X1", "waking");

  digitalWrite(PIN_BUZZER, HIGH);
  delay(80);
  digitalWrite(PIN_BUZZER, LOW);

  gasCalm = analogRead(PIN_GAS);
  lastGas = gasCalm;
  lastFlame = analogRead(PIN_FLAME);
  liveHold = 0;
  liveHoldAt = millis();
  showPhase = 0;
  showAt = millis();
  liveReadyAt = millis() + BOOT_MS + CAL_MS;
  lastDemo = digitalRead(PIN_DEMO);
  lastReset = digitalRead(PIN_RESET);
}

void loop() {
  pumpShow();
  readButtons();
  if (phoneStep == 0) {
    readLiveSensors();
    maybeLogGas();
  }
  setLights();
  setSound();
  setScreen();
  maybeWarnPhones();
  maybeBackupSms();
  pumpPhone();
}
