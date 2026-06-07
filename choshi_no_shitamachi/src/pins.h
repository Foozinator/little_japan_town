// pins.h — Choshi no Shitamachi, under construction (2026-06-07)
// Edit only when hardware changes. Agents: do not modify (see CLAUDE.md).
#pragma once
#include <stdint.h>

// GPIO
constexpr uint8_t PIN_AM2302 = 2;
constexpr uint8_t PIN_NEOPIXEL  = 3;
constexpr uint8_t PIN_TOUCH_IN  = 4;

// NeoPixel strip
constexpr uint8_t LAMP_ACCENT_COUNT = 4;

// Touch sensor
constexpr bool TOUCH_ACTIVE_HIGH = true;  // TTP223 momentary mode, CI high when touched

// I2C addresses
constexpr uint8_t I2C_EEPROM = 0x50;  // AT24C256 (A0-A2 grounded)
constexpr uint8_t I2C_INA219 = 0x40;  // INA219 (A0-A1 grounded)
constexpr uint8_t I2C_BH1750 = 0x23;  // BH1750 (ADDR pin low)

