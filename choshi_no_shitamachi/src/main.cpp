#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel pixel(1, 3, NEO_GRB + NEO_KHZ800);

const uint8_t EEPROM_ADDR = 0x50;
const char EXPECTED[] = "CHOSHI";
const uint8_t LEN = 6;

void eepromWrite(uint16_t addr, const uint8_t *data, uint8_t len) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((uint8_t)(addr >> 8));
  Wire.write((uint8_t)(addr & 0xFF));
  for (uint8_t i = 0; i < len; i++) Wire.write(data[i]);
  Wire.endTransmission();
  delay(10);
}

void eepromRead(uint16_t addr, uint8_t *buf, uint8_t len) {
  Wire.beginTransmission(EEPROM_ADDR);
  Wire.write((uint8_t)(addr >> 8));
  Wire.write((uint8_t)(addr & 0xFF));
  Wire.endTransmission();
  Wire.requestFrom(EEPROM_ADDR, len);
  for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  Wire.begin();
  pixel.begin();
  pixel.setPixelColor(0, 0);
  pixel.show();
  digitalWrite(LED_BUILTIN, LOW);

  eepromWrite(0x0000, (const uint8_t *)EXPECTED, LEN);

  uint8_t buf[LEN];
  eepromRead(0x0000, buf, LEN);

  Serial.print("wrote: ");
  Serial.println(EXPECTED);
  Serial.print("read:  ");
  for (uint8_t i = 0; i < LEN; i++) Serial.print((char)buf[i]);
  Serial.println();

  bool match = (memcmp(buf, EXPECTED, LEN) == 0);
  if (match) {
    Serial.println("OK");
    digitalWrite(LED_BUILTIN, HIGH);
  } else {
    Serial.println("MISMATCH");
    pixel.setPixelColor(0, pixel.Color(255, 0, 0));
    pixel.show();
  }
}

void loop() {}
