#pragma once

#define DEBUG 1
#if DEBUG
  #define DBG(...)   Serial.print(__VA_ARGS__)
  #define DBGLN(...) Serial.println(__VA_ARGS__)
#else
  #define DBG(...)
  #define DBGLN(...)
#endif

constexpr uint32_t BAUD_RATE          = 115200;
constexpr uint32_t SENSOR_INTERVAL_MS = 2000;
constexpr uint32_t TOUCH_DEBOUNCE_MS  = 20;
constexpr uint32_t CANDLE_MS          = 80;
