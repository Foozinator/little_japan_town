#include <Arduino.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>
#include <BH1750.h>
#include <Adafruit_INA238.h>

DHT dht(2, DHT22);
Adafruit_NeoPixel pixel(1, 3, NEO_GRB + NEO_KHZ800);
BH1750 lightMeter;
Adafruit_INA238 ina238;

const uint8_t WARM_R  = 255, WARM_G  = 160, WARM_B  = 60;
const uint8_t WHITE_R = 255, WHITE_G  = 255, WHITE_B  = 255;
const uint8_t COOL_R  = 180, COOL_G  = 220, COOL_B  = 255;

// Lux at which NeoPixel reaches full brightness
const float MAX_LUX = 500.0;

bool lastTouch = false;

void transmitReading() {
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();
  Serial.print("temp: ");
  Serial.print(tempC);
  Serial.print(" C  humidity: ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("bus: ");
  Serial.print(ina238.readBusVoltage(), 3);
  Serial.print(" V  current: ");
  Serial.print(ina238.readCurrent(), 4);
  Serial.println(" A");
}

void checkTouch() {
  bool touch = digitalRead(4);
  if (touch != lastTouch) {
    Serial.println(touch ? "touch: on" : "touch: off");
    lastTouch = touch;
  }
}

void fadeBetween(uint8_t r0, uint8_t g0, uint8_t b0,
                 uint8_t r1, uint8_t g1, uint8_t b1,
                 int steps, int msPerStep) {
  for (int i = 0; i <= steps; i++) {
    float lux = lightMeter.readLightLevel();
    if (lux < 0) lux = 0;
    float scale = constrain(lux, 0.0f, MAX_LUX) / MAX_LUX;

    uint8_t r = (uint8_t)(((int)r0 + ((int)r1 - (int)r0) * i / steps) * scale);
    uint8_t g = (uint8_t)(((int)g0 + ((int)g1 - (int)g0) * i / steps) * scale);
    uint8_t b = (uint8_t)(((int)b0 + ((int)b1 - (int)b0) * i / steps) * scale);
    pixel.setPixelColor(0, pixel.Color(r, g, b));
    pixel.show();
    checkTouch();
    delay(msPerStep);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(4, INPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  dht.begin();
  Wire.begin();
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
  ina238.begin(0x40, &Wire);
  pixel.begin();
  pixel.setPixelColor(0, 0);
  pixel.show();
}

void loop() {
  transmitReading();
  fadeBetween(WARM_R, WARM_G, WARM_B, WHITE_R, WHITE_G, WHITE_B, 200, 34); // ~6.8s
  fadeBetween(WHITE_R, WHITE_G, WHITE_B, COOL_R, COOL_G, COOL_B, 200, 34); // ~6.8s
  fadeBetween(COOL_R, COOL_G, COOL_B, WARM_R, WARM_G, WARM_B, 200, 34);   // ~6.8s
}
