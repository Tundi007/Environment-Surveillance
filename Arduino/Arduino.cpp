#include <Arduino.h>
#include <DHT.h>

// Pin assignments
constexpr uint8_t DHT_PIN = 2;        // Digital input for temperature/humidity sensor
constexpr uint8_t MQ135_PIN = A0;     // Analog gas sensor input
constexpr uint8_t TRIG_PIN = 4;       // Ultrasonic trigger
constexpr uint8_t ECHO_PIN = 5;       // Ultrasonic echo
constexpr uint8_t STATUS_LED = 13;    // On-board LED for quick status

// Sensor configuration
constexpr uint8_t DHT_TYPE = DHT22;
constexpr unsigned long SAMPLE_INTERVAL_MS = 1500;
constexpr unsigned long SERIAL_CMD_TIMEOUT_MS = 30;

static DHT dht(DHT_PIN, DHT_TYPE);

struct Sample {
  float temperatureC;
  float humidityPct;
  uint16_t gasRaw;
  float distanceCm;
  unsigned long capturedAtMs;
};

static Sample latest = {NAN, NAN, 0, NAN, 0};

static float readDistanceCm() {
  // Minimal blocking: limit pulseIn to ~25 ms (roughly 4.3 m max range)
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  const unsigned long duration = pulseIn(ECHO_PIN, HIGH, 25000UL);
  if (duration == 0) {
    return NAN;
  }

  // Speed of sound ~343 m/s -> 0.0343 cm/us; divide by 2 for round-trip
  return (duration * 0.0343F) * 0.5F;
}

static void refreshSampleIfStale() {
  const unsigned long now = millis();
  if (now - latest.capturedAtMs < SAMPLE_INTERVAL_MS) {
    return;
  }

  latest.temperatureC = dht.readTemperature();
  latest.humidityPct = dht.readHumidity();
  latest.gasRaw = static_cast<uint16_t>(analogRead(MQ135_PIN));
  latest.distanceCm = readDistanceCm();
  latest.capturedAtMs = now;
}

static bool readCommand(char *buffer, size_t len) {
  size_t idx = 0;
  const unsigned long start = millis();
  while (millis() - start < SERIAL_CMD_TIMEOUT_MS && idx + 1 < len) {
    while (Serial.available() && idx + 1 < len) {
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

static void emitJson(const Sample &s) {
  // Compact JSON to keep serial transfers quick for the ESP-01
  char payload[160];
  const int written = snprintf(
      payload, sizeof(payload),
      "{\"deviceId\":\"nano-01\","
      "\"millis\":%lu,"
      "\"temperature\":%.2f,"
      "\"humidity\":%.2f,"
      "\"gas\":%u,"
      "\"distance\":%.2f}",
      s.capturedAtMs,
      static_cast<double>(s.temperatureC),
      static_cast<double>(s.humidityPct),
      s.gasRaw,
      static_cast<double>(s.distanceCm));

  if (written > 0 && static_cast<size_t>(written) < sizeof(payload)) {
    Serial.println(payload);
  }
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(MQ135_PIN, INPUT);
  pinMode(STATUS_LED, OUTPUT);

  Serial.begin(115200);
  dht.begin();
  digitalWrite(STATUS_LED, HIGH); // quick power-on indicator
  delay(50);
  digitalWrite(STATUS_LED, LOW);
}

void loop() {
  refreshSampleIfStale();

  char cmd[12];
  if (readCommand(cmd, sizeof(cmd))) {
    if (strcmp(cmd, "DATA") == 0 || strcmp(cmd, "data") == 0) {
      digitalWrite(STATUS_LED, HIGH);
      emitJson(latest);
      digitalWrite(STATUS_LED, LOW);
    } else if (strcmp(cmd, "PING") == 0 || strcmp(cmd, "ping") == 0) {
      Serial.println(F("PONG"));
    }
  }
}
