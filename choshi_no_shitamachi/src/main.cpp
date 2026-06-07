#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_INA219.h>
#include <BH1750.h>
#include <DHT.h>

#include "pins.h"
#include "config.h"

Adafruit_NeoPixel strip(LAMP_ACCENT_COUNT, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Adafruit_INA219   ina219(I2C_INA219);
BH1750            bh1750;
DHT               dht(PIN_AM2302, DHT22);

static bool     ina219Ok_   = false;
static bool     bh1750Ok_   = false;
static bool     touchLast_  = false;
static uint32_t touchFired_ = 0;
static uint32_t sensorLast_ = 0;

// Touch cycles through these brightness levels: full → half → dim → off → full
static const uint8_t BRIGHT[] = { 255, 128, 26, 0 };
static uint8_t brightIdx_     = 0;

static void readEepromString() {
  Wire.beginTransmission(I2C_EEPROM);
  Wire.write((uint8_t)0);
  Wire.write((uint8_t)0);
  if (Wire.endTransmission() != 0) {
    DBGLN(F("EEPROM not found"));
    return;
  }
  Wire.requestFrom((uint8_t)I2C_EEPROM, (uint8_t)50);
  char buf[51];
  uint8_t i = 0;
  while (Wire.available() && i < 50) {
    uint8_t c = (uint8_t)Wire.read();
    if (c == 0) break;
    buf[i++] = (char)c;
  }
  buf[i] = '\0';
  DBG(F("EEPROM[0]: \""));
  DBG(buf);
  DBGLN(F("\""));
}

static void showColors() {
  uint8_t b = BRIGHT[brightIdx_];
  strip.setPixelColor(0, b, 0, 0);    // R
  strip.setPixelColor(1, 0, b, 0);    // G
  strip.setPixelColor(2, 0, 0, b);    // B
  strip.setPixelColor(3, b, b, b);    // W
  strip.show();
}

void setup() {
  Serial.begin(BAUD_RATE);
  Wire.begin();
  pinMode(PIN_TOUCH_IN, INPUT);

  strip.begin();
  strip.clear();
  strip.show();

  readEepromString();

  ina219Ok_ = ina219.begin();
  DBGLN(ina219Ok_ ? F("INA219 OK") : F("INA219 not found"));

  bh1750Ok_ = bh1750.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  DBGLN(bh1750Ok_ ? F("BH1750 OK") : F("BH1750 not found"));

  dht.begin();
  DBGLN(F("DHT begin"));

  showColors();
  DBGLN(F("ready"));
}

void loop() {
  uint32_t now = millis();

  // Rising-edge touch detection with debounce
  bool touchNow = (digitalRead(PIN_TOUCH_IN) == HIGH) == TOUCH_ACTIVE_HIGH;
  if (touchNow && !touchLast_ && (now - touchFired_) >= TOUCH_DEBOUNCE_MS) {
    touchFired_ = now;
    brightIdx_  = (brightIdx_ + 1) % sizeof(BRIGHT);
    showColors();
    DBG(F("touch b="));
    DBGLN(BRIGHT[brightIdx_]);
  }
  touchLast_ = touchNow;

  // Sensor reads every SENSOR_INTERVAL_MS
  if ((now - sensorLast_) >= SENSOR_INTERVAL_MS) {
    sensorLast_ = now;
    if (ina219Ok_) {
      DBG(F("V="));
      DBG(ina219.getBusVoltage_V());
      DBG(F(" mA="));
      DBGLN(ina219.getCurrent_mA());
    }
    if (bh1750Ok_) {
      DBG(F("lux="));
      DBGLN(bh1750.readLightLevel());
    }
    float temp = dht.readTemperature();
    float hum  = dht.readHumidity();
    if (!isnan(temp) && !isnan(hum)) {
      DBG(F("C="));   DBG(temp);
      DBG(F(" RH=")); DBGLN(hum);
    } else {
      DBGLN(F("DHT read failed"));
    }
  }
}
