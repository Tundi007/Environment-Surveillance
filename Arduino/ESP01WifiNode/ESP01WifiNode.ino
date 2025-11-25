#include <ESP8266WiFi.h>

// ---- WiFi settings (replace with your own) ----
const char *ssid = "YOUR_AP_SSID";
const char *password = "YOUR_AP_PASSWORD";

// ---- Endpoint (hub) settings ----
const char *endpointHost = "192.168.4.1"; // IP of the ESP01 hub (SoftAP gateway by default)
const uint16_t endpointPort = 5050;

// ---- Sensor + timing ----
const uint8_t mq135Pin = A0;
const unsigned long sendIntervalMs = 5000;

WiFiClient client;
unsigned long lastSend = 0;

bool ensureWifiConnected() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  WiFi.begin(ssid, password);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(250);
  }
  return WiFi.status() == WL_CONNECTED;
}

bool ensureEndpointConnected() {
  if (client.connected()) {
    return true;
  }
  client.stop();
  return client.connect(endpointHost, endpointPort);
}

void setup() {
  Serial.begin(115200);
  delay(10);
  WiFi.mode(WIFI_STA);
  ensureWifiConnected();
}

void loop() {
  if (!ensureWifiConnected()) {
    delay(500);
    return;
  }

  unsigned long now = millis();
  if (now - lastSend < sendIntervalMs) {
    return;
  }
  lastSend = now;

  int rawValue = analogRead(mq135Pin); // 0-1023 on ESP8266 ADC

  if (!ensureEndpointConnected()) {
    return;
  }

  // Simple line protocol: MQ135:<raw>\n
  client.print("MQ135:");
  client.println(rawValue);

  // Close after each send so the server can take over when it needs to fetch data.
  client.flush();
  client.stop();
}
