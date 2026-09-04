/*
  SIM800L only test. Same pin map. No gas, no LCD, no Demo.

  Wire first (light must blink):
    Cell plus  -> SIM VCC     (never Uno 5V)
    Cell minus -> SIM GND     straight black wire, NO resistor
    Uno GND    -> SIM GND     same drain, NO resistor
    SIM TXD    -> Uno D5      straight
    SIM RXD    -> Uno D6      through 10k, extra 10k from that joint to GND rail
    RST empty. Antenna on. SIM seated.

  Arduino IDE:
    1. File -> Open -> this folder sim800l_test
    2. Tools -> Board -> Arduino Uno
    3. Upload
    4. Tools -> Serial Monitor, 9600, "Both NL & CR"

  Watch the window. You want the word OK.
  Then type t  and press Enter = test text to the owner.
  Then type c  and press Enter = test call (rings ~10 seconds).
*/

#include <SoftwareSerial.h>

const int PIN_SIM_RX = 5;  // Uno listens. Wire to SIM TXD.
const int PIN_SIM_TX = 6;  // Uno talks. Wire to SIM RXD through 10k.

SoftwareSerial sim(PIN_SIM_RX, PIN_SIM_TX);

const char* OWNER = "+233557164067";

char line[48];
byte lineLen = 0;

void drainSim(unsigned long ms) {
  unsigned long t = millis();
  while (millis() - t < ms) {
    while (sim.available()) Serial.write(sim.read());
  }
}

void ask(const char* cmd, unsigned long waitMs) {
  Serial.print(F(">> "));
  Serial.println(cmd);
  sim.println(cmd);
  drainSim(waitMs);
}

void sendTestSms() {
  Serial.println(F("--- test SMS ---"));
  ask("AT+CMGF=1", 1000);
  Serial.print(F(">> AT+CMGS=\""));
  Serial.print(OWNER);
  Serial.println('"');
  sim.print("AT+CMGS=\"");
  sim.print(OWNER);
  sim.println('"');
  drainSim(1500);
  sim.print("AeroGuard SIM test. If you get this, the chip works.");
  sim.write(26);
  Serial.println(F(">> (message + Ctrl+Z)"));
  drainSim(15000);
  Serial.println(F("--- SMS command done ---"));
}

void makeTestCall() {
  Serial.println(F("--- test call ---"));
  Serial.print(F(">> ATD"));
  Serial.print(OWNER);
  Serial.println(';');
  sim.print("ATD");
  sim.print(OWNER);
  sim.println(';');
  drainSim(10000);
  ask("ATH", 1500);
  Serial.println(F("--- hang up ---"));
}

void setup() {
  Serial.begin(9600);
  sim.begin(9600);
  delay(300);

  Serial.println();
  Serial.println(F("SIM800L ONLY TEST"));
  Serial.println(F("GND = straight wire. No 10k in SIM GND."));
  Serial.println(F("LED must blink. If dark, stop and fix power."));
  Serial.println();

  for (int i = 0; i < 8; i++) {
    ask("AT", 700);
  }
  ask("ATE0", 500);
  ask("AT+CPIN?", 1000);
  ask("AT+CSQ", 1000);
  ask("AT+CREG?", 1000);
  ask("AT+COPS?", 1500);

  Serial.println();
  Serial.println(F("Want OK after AT, and +CREG: 0,1 or 0,5 (on network)."));
  Serial.println(F("Type t then Enter = test SMS"));
  Serial.println(F("Type c then Enter = test call"));
  Serial.println(F("Or type any AT command, like AT+CSQ"));
  Serial.println();
}

void loop() {
  while (sim.available()) Serial.write(sim.read());

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      line[lineLen] = 0;
      if (lineLen == 1 && (line[0] == 't' || line[0] == 'T')) {
        sendTestSms();
      } else if (lineLen == 1 && (line[0] == 'c' || line[0] == 'C')) {
        makeTestCall();
      } else if (lineLen > 0) {
        ask(line, 1500);
      }
      lineLen = 0;
    } else if (lineLen < 46) {
      line[lineLen++] = c;
    }
  }
}
