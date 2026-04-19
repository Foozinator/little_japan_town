# Little Japan Town

Arduino and ESP8266 enhancements to miniature flat-pack Japanese shop models.

The models are small laser-cut wooden kits. This project adds lighting, sensing, and wireless control to bring them to life on a shelf or diorama layout.

## Projects

### choshi_no_shitamachi
PlatformIO project targeting an Arduino Nano (ATmega328). Starting point for shop lighting and sensor integration.

### cornerstone
KiCad schematic and PCB layout for a custom carrier board — consolidates power regulation and wiring for the shop electronics.

## Hardware

Key components in [`parts_bin.md`](parts_bin.md):

- **Arduino Nano × 5** — main microcontroller for most shops
- **NodeMCU Amica × 2, Heltec HTIT-W8266 × 1, ESP-01 × 1** — ESP8266-based WiFi nodes for wireless control or monitoring
- **AOSONG AM2302** — DHT22 temperature/humidity sensor
- **Dallas DS18B20 × 3** — 1-Wire temperature sensors; multiple units share one data line
- **RCWL-0516** — microwave Doppler motion sensor (detects through walls/facades)
- **Sharp GP2D120X** — IR proximity sensor, 4–30 cm range
- **OKI-78SR-3.3 × 2** — efficient 3.3 V switching regulators for the ESP8266 modules
- **MOC3043 / MOC3023** — optoisolated triac drivers for any mains-voltage lighting effects

## Tools

- [PlatformIO](https://platformio.org/) for Arduino firmware
- [KiCad](https://www.kicad.org/) for PCB design
