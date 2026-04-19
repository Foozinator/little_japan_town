#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_NeoPixel.h>

DHT dht(2, DHT22);
Adafruit_NeoPixel pixel(1, 3, NEO_GRB + NEO_KHZ800);

const uint8_t WARM_R = 255;
const uint8_t WARM_G = 160;
const uint8_t WARM_B = 60;
const int FADE_STEPS = 200;
const int FADE_MS = 10; // 200 * 10ms = 2 seconds

void transmitReading() {
  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();
  Serial.print("temp: ");
  Serial.print(tempC);
  Serial.print(" C  humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}

void fadePixel(bool in) {
  for (int i = 0; i <= FADE_STEPS; i++) {
    int step = in ? i : (FADE_STEPS - i);
    pixel.setPixelColor(0, pixel.Color(
      WARM_R * step / FADE_STEPS,
      WARM_G * step / FADE_STEPS,
      WARM_B * step / FADE_STEPS
    ));
    pixel.show();
    delay(FADE_MS);
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  dht.begin();
  pixel.begin();
  pixel.setPixelColor(0, 0);
  pixel.show();
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  transmitReading();
  fadePixel(true);

  digitalWrite(LED_BUILTIN, LOW);
  delay(60000);

  transmitReading();
  fadePixel(false);
}
