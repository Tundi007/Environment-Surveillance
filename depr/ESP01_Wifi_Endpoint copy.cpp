//ESP 01 Wifi Module that Conencts Other Modules to the Nano Board and the MCU to the Internet
//TODO these libraries aren't imported in the project, fix if it is not ok.
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

constexpr char WIFI_SSID[] = "YOUR_WIFI_SSID";
constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
constexpr char WEB_HOST[] = "192.168.1.100";
constexpr uint16_t WEB_PORT = 80;
constexpr char WEB_PATH[] = "/telemetry";

constexpr unsigned long SERIAL_TIMEOUT_MS = 75;
constexpr size_t BUFFER_SIZE = 160;

static void connectWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint8_t retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries++ < 60) {
    delay(25);
  }
}

static bool readSerialPayload(char *buffer, size_t bufferLen) {
  size_t idx = 0;
  const unsigned long start = millis();

  while (millis() - start < SERIAL_TIMEOUT_MS && idx + 1 < bufferLen) {
    while (Serial.available() && idx + 1 < bufferLen) {
      const int incoming = Serial.read();
      if (incoming < 0) {
        continue;
      }

      if (incoming == '\n' || incoming == '\r') {
        buffer[idx] = '\0';
        return idx > 0;
      }

      buffer[idx++] = static_cast<char>(incoming);
    }
  }

  buffer[idx] = '\0';
  return idx > 0;
}

static bool requestMicrocontroller(char *buffer, size_t bufferLen) {
  Serial.write("DATA\n");
  return readSerialPayload(buffer, bufferLen);
}

static bool postToWeb(const char *payload) {
  WiFiClient client;
  if (!client.connect(WEB_HOST, WEB_PORT)) {
    return false;
  }

  const size_t payloadLen = strlen(payload);

  char header[128];
  const int headerLen = snprintf(
      header, sizeof(header),
      "POST %s HTTP/1.1\r\n"
      "Host: %s\r\n"
      "Content-Type: application/json\r\n"
      "Connection: close\r\n"
      "Content-Length: %u\r\n"
      "\r\n",
      WEB_PATH, WEB_HOST, static_cast<unsigned>(payloadLen));

  if (headerLen <= 0 || static_cast<size_t>(headerLen) >= sizeof(header)) {
    return false;
  }

  client.write(reinterpret_cast<const uint8_t *>(header), headerLen);
  client.write(reinterpret_cast<const uint8_t *>(payload), payloadLen);
  client.flush();
  client.stop();
  return true;
}

void setup() {
  Serial.begin(115200);
  connectWifi();
}

void loop() {
  connectWifi();

  char payload[BUFFER_SIZE];
  if (requestMicrocontroller(payload, sizeof(payload))) {
    postToWeb(payload);
  }

  delay(500);
}
