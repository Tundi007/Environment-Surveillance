#include <ESP8266WiFi.h>
#include <WiFiClient.h>

constexpr char WIFI_SSID[] = "YOUR_WIFI_SSID";
constexpr char WIFI_PASSWORD[] = "YOUR_WIFI_PASSWORD";
constexpr char ENDPOINT_HOST[] = "192.168.1.100";
constexpr uint16_t ENDPOINT_PORT = 80;
constexpr char ENDPOINT_PATH[] = "/telemetry";

constexpr uint8_t MQ135_ANALOG_PIN = A0;  // ADC input on ESP-01S
constexpr uint8_t MQ135_DIGITAL_PIN = 2;  // GPIO2 on ESP-01
constexpr unsigned long PUSH_INTERVAL_MS = 1200;

struct MQ135Sample {
  uint16_t analogValue;
  uint8_t digitalState;
};

static void connectWifiFast() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint8_t retries = 0;
  while (WiFi.status() != WL_CONNECTED && retries++ < 50) {
    delay(20);
  }
}

static MQ135Sample readMQ135() {
  MQ135Sample sample;
  sample.analogValue = analogRead(MQ135_ANALOG_PIN);
  sample.digitalState = static_cast<uint8_t>(digitalRead(MQ135_DIGITAL_PIN));
  return sample;
}

static bool sendSample(const MQ135Sample &sample) {
  WiFiClient client;
  client.setNoDelay(true);

  if (!client.connect(ENDPOINT_HOST, ENDPOINT_PORT)) {
    return false;
  }

  char payload[64];
  const int payloadLen = snprintf(
      payload, sizeof(payload),
      "{\"mq135\":{\"analog\":%u,\"digital\":%u}}",
      sample.analogValue, sample.digitalState);

  if (payloadLen <= 0 || payloadLen >= static_cast<int>(sizeof(payload))) {
    client.stop();
    return false;
  }

  char header[144];
  const int headerLen = snprintf(
      header, sizeof(header),
      "POST %s HTTP/1.1\r\n"
      "Host: %s\r\n"
      "Content-Type: application/json\r\n"
      "Connection: close\r\n"
      "Content-Length: %d\r\n"
      "\r\n",
      ENDPOINT_PATH, ENDPOINT_HOST, payloadLen);

  if (headerLen <= 0 || headerLen >= static_cast<int>(sizeof(header))) {
    client.stop();
    return false;
  }

  const bool headerOk = client.write(reinterpret_cast<const uint8_t *>(header), headerLen) == headerLen;
  const bool bodyOk = client.write(reinterpret_cast<const uint8_t *>(payload), payloadLen) == payloadLen;
  client.stop();
  return headerOk && bodyOk;
}

void setup() {
  pinMode(MQ135_DIGITAL_PIN, INPUT);
  connectWifiFast();
}

void loop() {
  connectWifiFast();

  static unsigned long lastPush = 0;
  const unsigned long now = millis();
  if (now - lastPush >= PUSH_INTERVAL_MS) {
    const MQ135Sample sample = readMQ135();
    sendSample(sample);
    lastPush = now;
  }

  delay(40);
}
