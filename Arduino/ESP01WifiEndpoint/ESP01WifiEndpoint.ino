#include <ESP8266WiFi.h>

// ---- SoftAP settings (node connects here) ----
const char *apSsid = "YOUR_AP_SSID";
const char *apPassword = "YOUR_AP_PASSWORD";

// ---- Server settings ----
const uint16_t listenPort = 5050;
WiFiServer server(listenPort);
WiFiClient activeClient;

// Forward incoming lines to the Nano over UART and back to the TCP client.
const unsigned long clientTimeoutMs = 30000;
unsigned long lastClientActivity = 0;

// Buffer for lines coming from the Nano.
char serialLine[96];
uint8_t serialLen = 0;

void setup() {
  Serial.begin(9600); // UART to Arduino Nano
  delay(10);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(apSsid, apPassword);
  server.begin();
}

void forwardLineToClient(const char *line) {
  if (activeClient && activeClient.connected() && line[0] != '\0') {
    activeClient.println(line);
    lastClientActivity = millis();
  }
}

void handleClientData() {
  while (activeClient.connected() && activeClient.available()) {
    String line = activeClient.readStringUntil('\n');
    line.trim();
    lastClientActivity = millis();
    if (line.length() == 0) {
      continue;
    }
    // Pass the raw line to the Nano.
    Serial.println(line);
  }
}

void handleSerialData() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      serialLine[serialLen] = '\0';
      forwardLineToClient(serialLine);
      serialLen = 0;
    } else if (c != '\r') {
      if (serialLen < sizeof(serialLine) - 1) {
        serialLine[serialLen++] = c;
      }
    }
  }
}

void loop() {
  WiFiClient nextClient = server.available();
  if (nextClient) {
    if (activeClient && activeClient.connected()) {
      activeClient.stop();
    }
    activeClient = nextClient;
    lastClientActivity = millis();
  }

  if (activeClient && activeClient.connected()) {
    handleClientData();
    handleSerialData();
    if (millis() - lastClientActivity > clientTimeoutMs) {
      activeClient.stop();
    }
  }
}
