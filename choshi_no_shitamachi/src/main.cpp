#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>

DHT dht(2, DHT22);
Adafruit_NeoPixel pixel(1, 3, NEO_GRB + NEO_KHZ800);

// Warm white at ~75%
const uint8_t WARM_R = 191;
const uint8_t WARM_G = 120;
const uint8_t WARM_B = 45;

bool lastTouch = false;

void transmitReading() {
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();
  Serial.print("temp: ");
  Serial.print(tempC);
  Serial.print(" C  humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
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
    pixel.setPixelColor(0, pixel.Color(
      (uint8_t)((int)r0 + ((int)r1 - (int)r0) * i / steps),
      (uint8_t)((int)g0 + ((int)g1 - (int)g0) * i / steps),
      (uint8_t)((int)b0 + ((int)b1 - (int)b0) * i / steps)
    ));
    pixel.show();
    delay(msPerStep);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(4, INPUT);
  dht.begin();
  pixel.begin();
  pixel.setPixelColor(0, 0);
  pixel.show();
}

void loop() {
  checkTouch();
  digitalWrite(LED_BUILTIN, HIGH);
  transmitReading();
  fadeBetween(0, 0, 0, WARM_R, WARM_G, WARM_B, 200, 30);         // 6s: off → warm white

  checkTouch();
  digitalWrite(LED_BUILTIN, LOW);
  delay(60000);

  checkTouch();
  transmitReading();
  fadeBetween(WARM_R, WARM_G, WARM_B, 0, 0, 0, 200, 30);         // 6s: warm white → off
}
