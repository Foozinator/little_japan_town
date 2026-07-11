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

// Lamp modes: tap cycles candle → white → off → candle ...
enum LampMode : uint8_t { MODE_CANDLE = 0, MODE_WHITE = 1, MODE_OFF = 2 };
static LampMode mode_ = MODE_OFF;

// Candle flicker state — one brightness per pixel, smoothed random walk
static uint8_t  candleBright_[LAMP_ACCENT_COUNT];
static uint32_t candleLast_ = 0;

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

static void setAllPixels(uint8_t r, uint8_t g, uint8_t b) {
  for (uint8_t i = 0; i < LAMP_ACCENT_COUNT; i++)
    strip.setPixelColor(i, r, g, b);
  strip.show();
}

// Warm candle color at brightness b: full warm orange, dims naturally
static void setCandlePixel(uint8_t i, uint8_t b) {
  strip.setPixelColor(i, b, (uint8_t)((uint16_t)b * 2 / 5), (uint8_t)(b / 32));
}

static void updateCandle(uint32_t now) {
  if ((now - candleLast_) < CANDLE_MS) return;
  candleLast_ = now;
  for (uint8_t i = 0; i < LAMP_ACCENT_COUNT; i++) {
    // Smooth random walk: bias 3:1 toward current, drift toward random target
    uint8_t tgt = (uint8_t)random(100, 255);
    candleBright_[i] = (uint8_t)(((uint16_t)candleBright_[i] * 3 + tgt) / 4);
    setCandlePixel(i, candleBright_[i]);
  }
  strip.show();
}

static void applyMode() {
  switch (mode_) {
    case MODE_CANDLE:
      for (uint8_t i = 0; i < LAMP_ACCENT_COUNT; i++) {
        candleBright_[i] = 180;
        setCandlePixel(i, 180);
      }
      strip.show();
      break;
    case MODE_WHITE:
      setAllPixels(255, 255, 255);
      break;
    case MODE_OFF:
      setAllPixels(0, 0, 0);
      break;
  }
  DBG(F("mode="));
  DBGLN(mode_);
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

  applyMode();
  DBGLN(F("ready"));
}

void loop() {
  uint32_t now = millis();

  // Fire on any edge — handles TTP223 in toggle mode (each tap = one edge)
  bool touchNow = (digitalRead(PIN_TOUCH_IN) == HIGH) == TOUCH_ACTIVE_HIGH;
  if (touchNow != touchLast_ && (now - touchFired_) >= TOUCH_DEBOUNCE_MS) {
    touchFired_ = now;
    mode_ = (LampMode)((uint8_t)(mode_ + 1) % 3);
    applyMode();
  }
  touchLast_ = touchNow;

  if (mode_ == MODE_CANDLE) updateCandle(now);

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
