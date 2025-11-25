#include <EEPROM.h>
#include <SoftwareSerial.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

// Pins used to talk to the ESP01 hub
const uint8_t ESP_RX = 10; // Nano RX 10 (to ESP TX)
const uint8_t ESP_TX = 11; // Nano TX on 11 (to ESP RX)
SoftwareSerial espSerial(ESP_RX, ESP_TX);

// Simple circular buffer in EEPROM
const uint16_t RECORD_CAP = 500;      // storage cap
const uint16_t RECORD_BYTES = 2;      // storage size
const uint16_t META_NEXT_ADDR = 0;    // 0-1 store nextIndex
const uint16_t META_COUNT_ADDR = 2;   // 2-3 store recordCount
const uint16_t START_ADDR = 4;        // data starts at byte 4 (leaves 24 bytes headroom in 1KB EEPROM)

uint16_t nextIndex = 0;
uint16_t recordCount = 0;

char lineBuf[48];
uint8_t lineLen = 0;

uint16_t readUint16(int addr) {
  return EEPROM.read(addr) | (uint16_t(EEPROM.read(addr + 1)) << 8);
}

void writeUint16(int addr, uint16_t value) {
  EEPROM.update(addr, value & 0xFF);
  EEPROM.update(addr + 1, (value >> 8) & 0xFF);
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

void storeReading(uint16_t value) {
  uint16_t slot = nextIndex % RECORD_CAP;
  int addr = START_ADDR + slot * RECORD_BYTES;
  EEPROM.update(addr, lowByte(value));
  EEPROM.update(addr + 1, highByte(value));

  nextIndex = (slot + 1) % RECORD_CAP;
  if (recordCount < RECORD_CAP) {
    recordCount++;
  }
  persistMeta();

  Serial.print("Stored MQ135 raw ");
  Serial.print(value);
  Serial.print(" at slot ");
  Serial.println((nextIndex + RECORD_CAP - 1) % RECORD_CAP);
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
    int addr = START_ADDR + slot * RECORD_BYTES;
    uint16_t value = readUint16(addr);

    espSerial.print("DATA:");
    espSerial.println(value);
  }

  clearBuffer();
  espSerial.println("END");
}

void handleLine(const char *line) {
  if (line[0] == '\0') {
    return;
  }

  // "MQ135:<value>"
  if (strncasecmp(line, "MQ135:", 6) == 0 || isdigit(line[0])) {
    const char *payload = line;
    if (strncasecmp(line, "MQ135:", 6) == 0) {
      payload = line + 6;
    }
    long v = strtol(payload, NULL, 10);
    if (v >= 0 && v <= 1023) {
      storeReading(static_cast<uint16_t>(v));
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
