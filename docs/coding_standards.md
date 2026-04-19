# Coding Standards — Shop Firmware

This document is the single reference for all shop firmware projects. It lives once at the workspace root; it is **not** copied into each project. Each shop project references it from its own `CLAUDE.md`.

## Scope and philosophy

One PlatformIO project per shop. One target board per project. Code is copied between shop projects, not shared through libraries. Duplication is accepted as the cost of project independence — each shop's firmware can be read, built, and reasoned about without any cross-project context.

Hard rules for agents are collected at the end of this document. If a change requires violating one, stop and surface the conflict.

## Target matrix

| | Cornerstone (1) | Child shop (N) |
|---|---|---|
| Board | Heltec HTIT-W8266 | Arduino Nano ATmega328P |
| MCU | ESP8266EX | ATmega328P |
| Logic level | 3.3V | 5V |
| SRAM | 80 KB | 2 KB |
| Flash | 4 MB | 32 KB |
| WiFi | yes | no |
| Serial ports | 2 hardware | 1 hardware (shared with USB), SoftwareSerial used for RS485 |

When a rule applies to only one target, tag it `[Nano only]` or `[ESP8266 only]`.

## Project layout

Every shop project uses this layout:

```
shop-<name>/
├── platformio.ini
├── CLAUDE.md                 # per-project context for the agent
├── src/
│   ├── main.cpp              # setup() and loop() only — no peripheral logic
│   ├── pins.h                # FROZEN — see "The frozen header"
│   ├── config.h              # software-side config (baud, poll rate, debug flag)
│   ├── peripherals/          # one .h/.cpp pair per peripheral driver
│   │   ├── touch.{h,cpp}
│   │   ├── light_white.{h,cpp}
│   │   └── neopixel.{h,cpp}
│   └── bus/
│       └── bus.{h,cpp}       # Modbus slave (Nano) or Modbus master + MQTT bridge (ESP8266)
```

Rules:
- No `.ino` files. PlatformIO uses `.cpp` and `.h`.
- `main.cpp` instantiates drivers and calls their `begin()` / `update()`. It contains no peripheral logic.
- No cross-project includes. If two shops need the same code, copy it.

## The frozen header: `pins.h`

`pins.h` captures the physical wiring of this specific shop. It is modified **only** by a human, **only** when the hardware changes.

Contents:
- Every GPIO pin used by this shop, as `constexpr uint8_t`.
- Every I2C address in use, as `constexpr uint8_t`.
- Every fixed hardware parameter set by the board (NeoPixel LED count, touch sensor active-high vs active-low, touch sensor mode, pull-up presence, etc.).

Example (per-shop, hand-edited):

```cpp
// pins.h — Ramen shop, board rev A (2026-04-12)
#pragma once
#include <stdint.h>

// GPIO pin assignments
constexpr uint8_t PIN_TOUCH_IN      = 4;   // TTP223 CI, momentary mode
constexpr uint8_t PIN_LIGHT_WHITE   = 5;   // PWM
constexpr uint8_t PIN_NEOPIXEL_DATA = 6;
constexpr uint8_t PIN_RS485_RX      = 2;   // SoftwareSerial
constexpr uint8_t PIN_RS485_TX      = 3;   // SoftwareSerial

// Fixed hardware parameters
constexpr uint8_t NEOPIXEL_COUNT    = 8;
constexpr bool    TOUCH_ACTIVE_HIGH = true;

// I2C addresses — none on this shop
```

**Agents: do not modify `pins.h`.** If code behavior suggests a pin is wrong, stop and report it. Do not "fix" the pin assignment.

## Naming

| Thing | Convention | Example |
|---|---|---|
| Files | `snake_case` | `light_white.cpp` |
| Classes / structs | `PascalCase` | `WhiteLight` |
| Functions / methods | `camelCase` | `beginBus()` |
| Constants (incl. pins.h) | `UPPER_SNAKE_CASE` | `PIN_TOUCH_IN` |
| Member variables | trailing underscore | `count_`, `lastMillis_` |
| MQTT topic segments | `lower_snake_case` | `light_white` |
| Modbus capability names | `lower_snake_case` | matches MQTT |

## Code style

- 2-space indent, no tabs.
- K&R braces (opening brace on same line).
- `#pragma once` in every header; no include guards.
- Include order: corresponding `.h` → stdlib → Arduino framework → third-party libs → project files. Blank line between groups.
- No `using namespace` in headers.
- Prefer `constexpr` over `#define` for constants.
- Prefer sized integer types (`uint8_t`, `int16_t`) over `int` for anything touching pins, registers, or the bus.
- No blocking `delay()` in `loop()`. Use `millis()`-based timers.

## Memory discipline [Nano only]

The Nano has 2 KB SRAM. Treat it as scarce.

- String literals go in flash: `Serial.println(F("bus timeout"));`.
- No `String` class. Use fixed-size `char` buffers.
- No dynamic allocation after `setup()` — no `new`, no `malloc`, no `std::vector`.
- Watch the PlatformIO-reported RAM usage. If it crosses ~75%, something is wrong.

The ESP8266 has headroom, but these habits carry over cleanly.

## Setup/loop shape

Every `main.cpp` follows this shape:

```cpp
#include "pins.h"
#include "config.h"
#include "peripherals/touch.h"
#include "peripherals/light_white.h"
#include "peripherals/neopixel.h"
#include "bus/bus.h"

Touch      touch;
WhiteLight whiteLight;
Neopixel   neopixel;
Bus        bus;

void setup() {
  Serial.begin(115200);
  touch.begin();
  whiteLight.begin();
  neopixel.begin();
  bus.begin();   // bus discovers peripherals via registration order
}

void loop() {
  touch.update();
  whiteLight.update();
  neopixel.update();
  bus.update();
}
```

## Peripheral driver pattern

Every peripheral driver is a class with:
- `begin()` — called once from `setup()`.
- `update()` — called every `loop()`, non-blocking.
- A header comment listing the Modbus registers it exposes and their order.

The bus layer walks drivers in construction order to build the Modbus descriptor. **Reordering driver construction or `begin()` calls changes the register map.** Do not reorder without explicitly flagging the consequence.

## Debug output

Single macro in `config.h`, so deployed builds can go silent:

```cpp
#define DEBUG 1
#if DEBUG
  #define DBG(...)   Serial.print(__VA_ARGS__)
  #define DBGLN(...) Serial.println(__VA_ARGS__)
#else
  #define DBG(...)
  #define DBGLN(...)
#endif
```

`Serial` is reserved for USB/debug. The RS485 bus uses SoftwareSerial on Nano (pins from `pins.h`) and `Serial1` on the Heltec.

## `platformio.ini` and dependencies

Single environment per project. Dependencies pinned to exact versions:

```ini
[env:nano]
platform  = atmelavr
board     = nanoatmega328
framework = arduino
monitor_speed = 115200
lib_deps =
  adafruit/Adafruit NeoPixel @ 1.12.3
  emelianov/modbus-esp8266 @ 4.1.0
```

If a range is used instead of an exact pin, leave a comment explaining why. Version bumps are their own commit, separate from feature changes.

## Build verification

Before any change is considered complete: `pio run` must succeed, with no new warnings. Note the reported flash and RAM usage in the commit message for non-trivial changes.

## Per-project `CLAUDE.md`

Each shop has its own `CLAUDE.md` at project root, containing:
- Target board.
- Shop name and current Modbus address.
- Current peripheral list and register layout.
- Hardware quirks of this specific shop (e.g., "touch sensor is in toggle mode, not momentary").
- Link back to this standards document.

The agent reads `CLAUDE.md` before making changes. The human updates it when hardware changes.

## Hard rules for agents

1. Do not modify `pins.h`.
2. Do not add dependencies without a version pin (or a comment explaining why not).
3. Do not share code between shop projects.
4. Do not introduce dynamic allocation after `setup()`.
5. Do not reorder peripheral construction or `begin()` calls without flagging the Modbus register-map consequence.
6. Do not use `delay()` in `loop()`.
7. If a change requires violating any of the above, stop and surface the conflict before proceeding.