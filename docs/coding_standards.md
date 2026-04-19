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
│   │   ├── lamp_white.{h,cpp}
│   │   └── lamp_accent.{h,cpp}
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
// pins.h — <shop name>, board rev A (<date>)
#pragma once
#include <stdint.h>

// GPIO pin assignments
constexpr uint8_t PIN_TOUCH_IN         = 4;   // TTP223 CI, momentary mode
constexpr uint8_t PIN_LAMP_WHITE       = 5;   // PWM
constexpr uint8_t PIN_LAMP_ACCENT_DATA = 6;   // NeoPixel data in
constexpr uint8_t PIN_RS485_RX         = 2;   // SoftwareSerial
constexpr uint8_t PIN_RS485_TX         = 3;   // SoftwareSerial

// Fixed hardware parameters
constexpr uint8_t LAMP_ACCENT_COUNT    = 8;   // NeoPixel pixel count
constexpr bool    TOUCH_ACTIVE_HIGH    = true;

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
#include "peripherals/lamp_white.h"
#include "peripherals/lamp_accent.h"
#include "bus/bus.h"

Touch      touch;
LampWhite  lampWhite;
LampAccent lampAccent;
Bus        bus;

void setup() {
  Serial.begin(115200);
  touch.begin();
  lampWhite.begin();
  lampAccent.begin();
  bus.begin();   // bus discovers peripherals via registration order
}

void loop() {
  touch.update();
  lampWhite.update();
  lampAccent.update();
  bus.update();
}
```

## Peripheral driver pattern

Every peripheral driver is a class with:
- `begin()` — called once from `setup()`.
- `update()` — called every `loop()`, non-blocking.
- A header comment listing the Modbus registers it exposes and their order.

The bus layer walks drivers in construction order to build the Modbus descriptor. **Reordering driver construction or `begin()` calls changes the register map.** Do not reorder without explicitly flagging the consequence.

## Touch input convention

Every shop has at least one `Touch` peripheral. Its Modbus representation is a single `uint8` register: a rising-edge counter.

- Increments on each `LOW → HIGH` transition of the touch input (assuming active-HIGH TTP223; for active-LOW wiring, increment on falling edge — set by `TOUCH_ACTIVE_HIGH` in `pins.h`).
- Rolls over at 255 → 0.
- Resets to 0 on boot.
- Firmware applies a 20 ms debounce window; no further action needed on the cornerstone side.

The cornerstone computes `delta = (current - last_seen) & 0xFF` and fires one "touched" event per count. A `current < last_seen` reading indicates the shop rebooted; the cornerstone resets its baseline and suppresses the event burst.

This design captures multiple presses between polls (up to 255 events) and is naturally robust to missed polls.

## Lamp control convention

Every shop has at least one lamp. All lamps — whether a single-channel white LED, a NeoPixel strip treated as a bulk color, or a single chōchin — present the same 6-register block.

### Block layout (per lamp, 6 consecutive holding registers)

| Offset | Field       | Type   | Notes |
|--------|-------------|--------|-------|
| +0     | mode        | uint8  | See theme IDs below |
| +1     | target_r    | uint8  | Target red component |
| +2     | target_g    | uint8  | Target green component |
| +3     | target_b    | uint8  | Target blue component |
| +4     | fade_ms_lo  | uint8  | Fade duration low byte |
| +5     | fade_ms_hi  | uint8  | Fade duration high byte |

One byte per register, upper byte reserved. Wasteful in wire bytes but dead simple to reason about and matches the descriptor format directly. At 9600 baud on the poll cadence we're using, bandwidth is not the constraint.

### Theme IDs

Every shop's firmware implements the same numbering:

| ID | Name         | Behavior |
|----|--------------|----------|
| 0  | STATIC       | Hold `target` color; `fade_ms` ignored. |
| 1  | FADE         | Linear fade from current color to `target` over `fade_ms` milliseconds, then hold. |
| 2  | COOL_WHITE   | Steady ~6500K white; `target` and `fade_ms` ignored. |
| 3  | WARM_WHITE   | Steady ~2700K white; `target` and `fade_ms` ignored. |
| 4  | FLUORESCENT  | Warmup flicker phase, then settle to cool-white. Shop handles internally; mode stays at FLUORESCENT. |
| 5  | CANDLE       | Ongoing warm-white flame flicker. |

Additional IDs may be defined per shop but must be numbered `≥ 64` to leave room for shared themes.

### Monochrome fallback

A lamp that cannot render a given theme falls back silently: brightness-from-RGB computed as `max(r, g, b)`, applied as PWM. For example:
- A single white LED asked for `(0, 0, 255)` lights to full brightness.
- A single white LED asked for `CANDLE` flickers the PWM output in the candle pattern with no color component.

The fallback is local to the shop — the cornerstone doesn't need to know whether a given lamp is color-capable.

### Atomic writes

The cornerstone updates a lamp block using Modbus function `0x10` (write multiple registers), writing all 6 registers in one transaction. The shop sees `(mode, target, fade_ms)` update atomically — no half-applied state, no risk of the fade being read against a stale mode.

Single-register writes to a lamp block are not defined behavior. The cornerstone MQTT bridge enforces this; the shop does not need to guard against it.

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