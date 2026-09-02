/*
  ============================================================
  AEROGUARD-X1 — ESP32 WiFi bridge
  Board: ESP32 DevKit (or compatible)

  Role:
   - Talks UART to the Arduino Uno (A1/A3 ↔ GPIO 16/17)
   - Joins home WiFi (or opens a fallback AP)
   - Serves live STATUS over HTTP so the phone can reach the
     device remotely on the same network (or via tunnel/cloud later)

  Wiring (3.3V logic on ESP32 side — required, do not skip):
   ESP32 GPIO16 (RX2)  <- Uno A3 (TX) via 10k + 20k divider
                         Uno A3 --[10k]-- GPIO16 --[20k]-- GND
                         (5V down to ~3.3V; never wire A3 straight to GPIO16)
   ESP32 GPIO17 (TX2)  -> Uno A1 (RX)   [straight wire; 3.3V is safe for Uno]
   ESP32 GND           -- Uno GND
   Power ESP32 from USB or 5V VIN separately

  Upload with Arduino IDE / PlatformIO (ESP32 Arduino core).
  Open Serial 115200 for the IP address, then:
    GET http://<ip>/status
    POST http://<ip>/vent/open
  ============================================================
*/

#include <WiFi.h>
#include <WebServer.h>

// --- set your home WiFi (leave blank to use AP-only) ---
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

const char* AP_SSID = "AeroGuard-X1";
const char* AP_PASS = "aeroguard";  // change for demos

HardwareSerial UnoSerial(2);  // UART2
WebServer server(80);

String lastStatus = "STATUS level=SAFE demo=0 gas=0 vent=APP";
String lastLevel = "SAFE";
int lastGas = 0;
bool lastDemo = false;

void setup() {
  Serial.begin(115200);
  delay(300);
  UnoSerial.begin(9600, SERIAL_8N1, 16, 17);  // RX=16, TX=17

  bool joined = false;
  if (WIFI_SSID[0] != '\0' && strcmp(WIFI_SSID, "YOUR_WIFI_SSID") != 0) {
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    Serial.print(F("WiFi connecting"));
    for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; i++) {
      delay(250);
      Serial.print('.');
    }
    Serial.println();
    joined = WiFi.status() == WL_CONNECTED;
  }

  if (joined) {
    Serial.print(F("Remote URL: http://"));
    Serial.println(WiFi.localIP());
  } else {
    WiFi.mode(WIFI_AP);
    WiFi.softAP(AP_SSID, AP_PASS);
    Serial.print(F("AP mode SSID="));
    Serial.print(AP_SSID);
    Serial.print(F("  http://"));
    Serial.println(WiFi.softAPIP());
  }

  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/vent/open", HTTP_POST, handleVentOpen);
  server.on("/vent/open", HTTP_GET, handleVentOpen);  // easy demo from browser
  server.begin();
  Serial.println(F("AeroGuard ESP32 bridge ready"));
}

void loop() {
  pollUno();
  server.handleClient();
}

void pollUno() {
  static String line;
  while (UnoSerial.available()) {
    char c = (char)UnoSerial.read();
    if (c == '\r') continue;
    if (c == '\n') {
      if (line.startsWith("STATUS ")) {
        lastStatus = line;
        parseStatus(line);
        Serial.println(line);
      } else if (line.startsWith("APP_CMD:")) {
        Serial.print(F("From Uno: "));
        Serial.println(line);
      }
      line = "";
    } else {
      line += c;
      if (line.length() > 120) line = "";
    }
  }
}

void parseStatus(const String& s) {
  int li = s.indexOf("level=");
  int di = s.indexOf("demo=");
  int gi = s.indexOf("gas=");
  if (li >= 0) {
    int end = s.indexOf(' ', li + 6);
    lastLevel = end < 0 ? s.substring(li + 6) : s.substring(li + 6, end);
  }
  if (di >= 0) lastDemo = s.charAt(di + 5) == '1';
  if (gi >= 0) lastGas = s.substring(gi + 4).toInt();
}

void handleRoot() {
  String html = F("<!doctype html><html><body style='font-family:sans-serif;padding:1.5rem'>"
                  "<h1>AeroGuard-X1</h1><p>ESP32 WiFi bridge</p>"
                  "<p><a href='/status'>/status</a> · "
                  "<a href='/vent/open'>open vents</a></p>"
                  "<pre>");
  html += lastStatus;
  html += F("</pre></body></html>");
  server.send(200, "text/html", html);
}

void handleStatus() {
  // Build JSON without nested quote-escapes (keeps sketch transport-safe).
  String q = String((char)34);
  String json = String("{") + q + "level" + q + ":" + q + lastLevel + q + ",";
  json += q + "demo" + q + ":" + String(lastDemo ? "true" : "false") + ",";
  json += q + "gas" + q + ":" + String(lastGas) + ",";
  json += q + "raw" + q + ":" + q + lastStatus + q + ",";
  json += q + "remote" + q + ":true}";
  server.send(200, "application/json", json);
}

void handleVentOpen() {
  UnoSerial.println("APP_CMD:VENT_OPEN");
  String q = String((char)34);
  String body = String("{") + q + "ok" + q + ":true," + q + "cmd" + q + ":" + q
                + "APP_CMD:VENT_OPEN" + q + "}";
  server.send(200, "application/json", body);
}
