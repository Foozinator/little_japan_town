| Component | Qty | Interface | Notes |
|---|---|---|---|
| Arduino Nano | x5 | USB/Digital/Analog | Unknown controller (CH340 vs FTDI matters for driver setup) |
| AOSONG AM2302 | x1 | Single-wire digital | DHT22 variant, pre-wired 3-pin connector, SN 17113CB16 (mfg week 13 2017), no breadboard wiring needed |
| Sharp GP2D120X | x1 | Analog | IR distance sensor, 4–30cm range, 3-pin JST connector, analog voltage output (nonlinear — needs lookup table or curve fitting) |
| NodeMCU Amica | x2 | WiFi (802.11b/g/n), Digital, Analog (1 pin) | ESP8266MOD, 3.3V logic, built-in USB-serial, 1x analog input (10-bit, 0–1V max — not 3.3V) |
| Heltec HTIT-W8266 | x1 | WiFi (802.11b/g/n), I2C (OLED), Digital, Analog | ESP8266EX, 3.3V logic, 0.91" 128x32 OLED (SSD1306), built-in LiPo charging circuit, same 1V A0 limit as NodeMCU |
| ESP-01 (ESP8266) | x1 | WiFi, 2x Digital GPIO | No USB — requires FTDI/CH340 adapter to program, GPIO0 must be grounded to enter flash mode, 3.3V only, limited I/O makes it best suited for dedicated single-purpose tasks |
| RCWL-0516 | x1 | Digital (single pin) | Microwave Doppler, 5–7m range, detects through non-metal obstacles, 4–28V input but 3.3V-compatible output, ~65mA active draw |
| Dallas DS18B20 | x3 | 1-Wire | -55–125°C range, ±0.5°C accuracy, unique 64-bit ID allows multiple sensors on a single wire, requires 4.7kΩ pull-up resistor, bare TO-92 |
| OKI-78SR-3.3 | x2 | Power | 3.3V/1.5A switching regulator, 7–36V input, ~82% efficiency, 7805-pinout compatible, no heatsink needed |
| Duemilanove | x1 | Digital, Analog, UART | ATmega328, fully Arduino-compatible, 5V logic |
| Netduino (rev.B) | x1 | Digital, Analog, UART (+ SD/Ethernet if Plus variant) | STM32, .NET Micro Framework — tooling largely dead, usable as generic STM32 board |
| Netduino Mini | x1 | Digital, UART, no USB | STM32F103, 3.3V, serial programming only, 24-pin DIP |
| MOC3043 | x1 | Digital (control side) | AC triac driver, zero-crossing detection built in, 6-DIP, isolates MCU from mains voltage — needs external triac (e.g. BTA16) for actual load switching |
| LM2596 module | x5 | Power | Adjustable DC-DC step-down, 4–40V in, 1.25–37V out, 3A max, adjust output via onboard trimmer — set and verify with multimeter before connecting anything |
| SRD-05VDC-SL-C module | x1 | Digital (active LOW typically) | 5V coil, 10A/250VAC contacts, onboard transistor driver, control signal from MCU pin — add flyback diode if using bare relay, module likely has one already |
| ACS711LC breakout (custom PCB) | x1 | Analog | Hall-effect current sensor, ±15.5A range, analog voltage output proportional to current, 3.3–5V supply, no isolation on the LC variant |
| MOC3023 | x4 | Digital (control side) | AC triac driver, no zero-crossing, 6-DIP, pairs with external triac for load switching — better for resistive loads than inductive |
| Futaba FP-S148 | x1 | 3-pin PWM input | Standard cheap servo |

+ a handful of random resistors, LEDs, capacitors, ribbon cables, pins and sockets