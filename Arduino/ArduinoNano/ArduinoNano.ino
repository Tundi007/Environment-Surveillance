#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Pins used to talk to the ESP01 hub
const uint8_t ESP_RX = 10; // Nano RX 10 (to ESP TX)
const uint8_t ESP_TX = 11; // Nano TX on 11 (to ESP RX)
SoftwareSerial espSerial(ESP_RX, ESP_TX);

struct ClimateSample {
  int16_t tempTenths;   // temperature in 0.1 C
  int16_t humTenths;    // humidity in 0.1 %RH
  uint16_t gasRaw;      // MQ-135 ADC 0-1023
};

// Simple circular buffer in EEPROM
const uint16_t RECORD_CAP = 150;               // number of climate samples we can hold
const uint16_t RECORD_BYTES = sizeof(ClimateSample);
const uint16_t META_NEXT_ADDR = 0;    // 0-1 store nextIndex
const uint16_t META_COUNT_ADDR = 2;   // 2-3 store recordCount
const uint16_t START_ADDR = 4;        // data starts at byte 4 (leaves 24 bytes headroom in 1KB EEPROM)

uint16_t nextIndex = 0;
uint16_t recordCount = 0;

char lineBuf[72];
uint8_t lineLen = 0;
uint16_t readUint16(int addr) {
  return EEPROM.read(addr) | (uint16_t(EEPROM.read(addr + 1)) << 8);
}

void writeUint16(int addr, uint16_t value) {
  EEPROM.update(addr, value & 0xFF);
  EEPROM.update(addr + 1, (value >> 8) & 0xFF);
}

void writeSample(uint16_t slot, const ClimateSample &s) {
  const int addr = START_ADDR + slot * RECORD_BYTES;
  EEPROM.put(addr, s);
}

ClimateSample readSample(uint16_t slot) {
  ClimateSample s;
  const int addr = START_ADDR + slot * RECORD_BYTES;
  EEPROM.get(addr, s);
  return s;
}

void persistMeta() {
  writeUint16(META_NEXT_ADDR, nextIndex);
  writeUint16(META_COUNT_ADDR, recordCount);
}

void loadMeta() {
  nextIndex = readUint16(META_NEXT_ADDR);
  recordCount = readUint16(META_COUNT_ADDR);
  if (nextIndex >= RECORD_CAP) {
    nextIndex = 0;
  }
  if (recordCount > RECORD_CAP) {
    recordCount = 0;
  }
}

void clearBuffer() {
  nextIndex = 0;
  recordCount = 0;
  persistMeta();
}

void storeSample(int16_t tempTenths, int16_t humTenths, uint16_t gasRaw) {
  ClimateSample sample = {tempTenths, humTenths, gasRaw};

  const uint16_t slot = nextIndex % RECORD_CAP;
  writeSample(slot, sample);

  nextIndex = (slot + 1) % RECORD_CAP;
  if (recordCount < RECORD_CAP) {
    recordCount++;
  }
  persistMeta();

  Serial.print("Stored sample T/H/G = ");
  Serial.print(tempTenths / 10.0, 1);
  Serial.print("/");
  Serial.print(humTenths / 10.0, 1);
  Serial.print("/");
  Serial.print(gasRaw);
  Serial.print(" at slot ");
  Serial.println(slot);
}

void sendAllReadings() {
  espSerial.print("BEGIN ");
  espSerial.println(recordCount);

  if (recordCount == 0) {
    espSerial.println("END");
    return;
  }

  uint16_t start = (nextIndex + RECORD_CAP - recordCount) % RECORD_CAP;
  for (uint16_t i = 0; i < recordCount; i++) {
    uint16_t slot = (start + i) % RECORD_CAP;
    ClimateSample sample = readSample(slot);

    espSerial.print("DATA:");
    espSerial.print(sample.tempTenths / 10.0, 1);
    espSerial.print(",");
    espSerial.print(sample.humTenths / 10.0, 1);
    espSerial.print(",");
    espSerial.println(sample.gasRaw);
  }

  clearBuffer();
  espSerial.println("END");
}

bool parseClimateTriple(const char *payload, int16_t &tempTenths, int16_t &humTenths, uint16_t &gasRaw) {
  // Work on a copy because strtok modifies the buffer.
  char buf[48];
  strncpy(buf, payload, sizeof(buf) - 1);
  buf[sizeof(buf) - 1] = '\0';

  char *tok = strtok(buf, ",");
  if (!tok) return false;
  double t = strtod(tok, NULL);

  tok = strtok(NULL, ",");
  if (!tok) return false;
  double h = strtod(tok, NULL);

  tok = strtok(NULL, ",");
  if (!tok) return false;
  long g = strtol(tok, NULL, 10);
  if (g < 0 || g > 1023) return false;

  tempTenths = static_cast<int16_t>(t * 10.0);
  humTenths = static_cast<int16_t>(h * 10.0);
  gasRaw = static_cast<uint16_t>(g);
  return true;
}

void handleLine(const char *line) {
  if (line[0] == '\0') {
    return;
  }

  // "MQ135:<value>"
  if (strncasecmp(line, "CLIMATE:", 8) == 0 || strncasecmp(line, "DATA:", 5) == 0) {
    const char *payload = line + (tolower(line[0]) == 'c' ? 8 : 5);
    int16_t tTenths = 0;
    int16_t hTenths = 0;
    uint16_t gas = 0;
    if (parseClimateTriple(payload, tTenths, hTenths, gas)) {
      storeSample(tTenths, hTenths, gas);
    }
    return;
  }

  if (strncasecmp(line, "MQ135:", 6) == 0 || isdigit(line[0])) {
    const char *payload = line;
    if (strncasecmp(line, "MQ135:", 6) == 0) {
      payload = line + 6;
    }
    long v = strtol(payload, NULL, 10);
    if (v >= 0 && v <= 1023) {
      // If only gas is provided, still store a full record with T/H set to 0.0.
      storeSample(0, 0, static_cast<uint16_t>(v));
    }
    return;
  }

  if (strcasecmp(line, "FETCH") == 0) {
    sendAllReadings();
    return;
  }
  if (strcasecmp(line, "CLEAR") == 0) {
    clearBuffer();
    espSerial.println("CLEARED");
    return;
  }
  if (strcasecmp(line, "PING") == 0) {
    espSerial.println("PONG");
    return;
  }
}

void setup() {
  Serial.begin(115200);   // USB
  espSerial.begin(9600);  // ESP01

  loadMeta();
  Serial.print("EEPROM next slot: ");
  Serial.println(nextIndex);
  Serial.print("Stored readings: ");
  Serial.println(recordCount);
}

void loop() {
  while (espSerial.available()) {
    char c = espSerial.read();
    if (c == '\n') {
      lineBuf[lineLen] = '\0';
      handleLine(lineBuf);
      lineLen = 0;
    } else if (c != '\r') {
      if (lineLen < sizeof(lineBuf) - 1) {
        lineBuf[lineLen++] = c;
      }
    }
  }
}
