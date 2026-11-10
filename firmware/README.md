# Firmware

This directory contains the Arduino firmware for the four controllers used in
the physical autonomous tram prototype.

## Controller overview

| Firmware | Controller role |
| --- | --- |
| [`tram_controller.ino`](tram_controller.ino) | Drives the tram motor, receives commands from the remote controller, reads the VL53L0X distance sensor and handles the reed-switch stop sequence. |
| [`remote_controller.ino`](remote_controller.ino) | Reads the joystick and control buttons, displays the startup graphic on the OLED and transmits commands to the tram over NRF24L01. |
| [`switch_controller.ino`](switch_controller.ino) | Reads RFID tags and moves the track-switch servo to the configured position. |
| [`traffic_lights_controller.ino`](traffic_lights_controller.ino) | Reads RFID tags and runs the two-direction traffic-light sequence. |

## Target hardware

The original prototype uses **Arduino Nano boards based on the ATmega328P**.
Each sketch is intended for a separate controller. Select the correct processor
variant for your Nano in the Arduino IDE; older boards may require **ATmega328P
(Old Bootloader)**.

## Required libraries

Install the following libraries using the Arduino IDE Library Manager before
compiling the sketches.

| Library | Used by | Notes |
| --- | --- | --- |
| `RF24` by TMRh20 | Tram, remote | NRF24L01 radio communication. |
| `VL53L0X` by Pololu | Tram | Time-of-flight distance sensor. |
| `Adafruit GFX Library` | Remote | OLED graphics dependency. |
| `Adafruit SSD1306` | Remote | SSD1306 OLED driver. The sketch uses the legacy `Adafruit_SSD1306 display(OLED_RESET)` constructor; a small constructor update may be needed with recent library versions. |
| `MFRC522` by GithubCommunity / miguelbalboa | Switch, traffic lights | RFID reader support. |
| `Servo` | Switch | Included with the Arduino AVR core. |
| `SPI` and `Wire` | Several controllers | Included with the Arduino AVR core. |

## Pin assignments

The tables below document the assignments present in the current source code.
Always compare them with the physical wiring and the schematics before applying
power.

### Tram controller

| Arduino Nano pin | Connected function | Defined in code as |
| --- | --- | --- |
| D2 | Configured as an output; reserved in the current sketch | — |
| D3 | TB6612FNG motor direction input | `a2` |
| D4 | TB6612FNG motor direction input | `a1` |
| D5 (PWM) | TB6612FNG motor-speed input | `pwmA` |
| D7 | NRF24L01 CE | `RF24 radio(7, 8)` |
| D8 | NRF24L01 CSN | `RF24 radio(7, 8)` |
| D11 | NRF24L01 MOSI (hardware SPI) | implicit |
| D12 | NRF24L01 MISO (hardware SPI) | implicit |
| D13 | NRF24L01 SCK (hardware SPI) | implicit |
| A2 | Reed-switch input | `gerk` |
| A3 | Low reference for the reed switch | `gerkGnd` |
| A4 | VL53L0X SDA (I2C) | implicit |
| A5 | VL53L0X SCL (I2C) | implicit |

The autonomous mode uses a **70 mm** obstacle threshold. When the reed switch
is activated, the current implementation runs the tram slowly for 2 seconds,
then stops it for 5 seconds. These intervals are blocking because they use
`delay()`.

### Remote controller

| Arduino Nano pin | Connected function | Defined in code as |
| --- | --- | --- |
| D2 | Autopilot button, active low with pull-up | — |
| D3 | Selects between the A7 and A6 joystick inputs | — |
| D4 | Stop button, active low with pull-up | — |
| D5 | OLED reset | `OLED_RESET` |
| D6 | Reverse-direction input with pull-up enabled by the sketch | — |
| D7 | NRF24L01 CE | `RF24 radio(7, 8)` |
| D8 | NRF24L01 CSN | `RF24 radio(7, 8)` |
| D11 | NRF24L01 MOSI (hardware SPI) | implicit |
| D12 | NRF24L01 MISO (hardware SPI) | implicit |
| D13 | NRF24L01 SCK (hardware SPI) | implicit |
| A4 | SSD1306 OLED SDA (I2C) | implicit |
| A5 | SSD1306 OLED SCL (I2C) | implicit |
| A6 | Alternate joystick input | — |
| A7 | Primary joystick input | `JOYSTICK_Y` |

The OLED is initialized at I2C address **`0x3C`**. Inputs D2, D4 and D6 use
the AVR pull-up behavior and are therefore active low where inverted in the
source. D3 must be held at a defined logic level by the controller wiring.

### Track-switch controller

| Arduino Nano pin | Connected function | Defined in code as |
| --- | --- | --- |
| D3 | Track-switch servo signal | `servo.attach(3)` |
| D9 | MFRC522 reset | `RST_PIN` |
| D10 | MFRC522 slave select | `SS_PIN` |
| D11 | MFRC522 MOSI (hardware SPI) | implicit |
| D12 | MFRC522 MISO (hardware SPI) | implicit |
| D13 | MFRC522 SCK (hardware SPI) | implicit |

The initial servo position is 110 degrees. Recognized RFID tags move it to
either 57 or 127 degrees. Calibrate these angles for the mechanical limits of
your own switch before continuous operation.

### Traffic-light controller

| Arduino Nano pin | Connected function | Defined in code as |
| --- | --- | --- |
| D3 | Direction 1 amber light | `ORG1` |
| D4 | Direction 1 green light | `GRN1` |
| D5 | Direction 1 red light | `RED1` |
| D6 | Direction 2 amber light | `ORG2` |
| D7 | Direction 2 green light | `GRN2` |
| D8 | Direction 2 red light | `RED2` |
| D9 | MFRC522 reset | `RST_PIN` |
| D10 | MFRC522 slave select | `SS_PIN` |
| D11 | MFRC522 MOSI (hardware SPI) | implicit |
| D12 | MFRC522 MISO (hardware SPI) | implicit |
| D13 | MFRC522 SCK (hardware SPI) | implicit |

After a recognized RFID tag is detected, the sketch uses a 1-second transition
phase and keeps the requested direction green for 10 seconds.

## NRF24L01 configuration

The tram and remote sketches must use the same pipe address:

```cpp
const uint64_t pipe = 0xE8E8F0F0E1LL;
```

Both controllers currently use CE pin D7 and CSN pin D8. The remote transmits
an array of four AVR `int` values:

| Array item | Meaning |
| --- | --- |
| `joystick[0]` | Motor command read from A7 or A6 |
| `joystick[1]` | Autopilot button state |
| `joystick[2]` | Emergency-stop button state |
| `joystick[3]` | Direction/reverse state |

Because this is a raw binary array, both radios must be connected to compatible
AVR-based controllers using the same data layout. If the protocol is later
ported to another architecture, replace the array with a packed structure that
uses fixed-width integer types.

NRF24L01 modules require a stable **3.3 V** supply. Add local decoupling near
the module and do not power it from 5 V.

## Configuring RFID tags

RFID UIDs are stored as decimal numbers directly in
`switch_controller.ino` and `traffic_lights_controller.ino`. To register your
own tags:

1. Temporarily enable `Serial.begin(9600)` in `setup()`.
2. Enable a `Serial.println(uidDec)` statement after the UID conversion loop.
3. Upload the temporary sketch and open the Serial Monitor at 9600 baud.
4. Scan every tag and record the printed decimal value.
5. Replace the example UID constants in the relevant `if` conditions.
6. Upload the final sketch and verify every route before attaching the servo to
   the switch mechanism.

The current conversion stores the UID in an AVR `unsigned long`, so it is
designed for four-byte tags. Use a byte-array comparison if tags with longer
UIDs must be supported.

## Recommended upload order

There is no strict software dependency between the four controllers, but the
following order makes commissioning safer and easier to diagnose:

1. **Traffic-light controller** — verify the normal signal state and each RFID
   trigger without connecting the tram motor.
2. **Track-switch controller** — verify the RFID mapping and servo angles with
   the mechanism unloaded.
3. **Tram controller** — lift the driven wheels from the track and verify stop,
   direction, obstacle detection and reed-switch behavior.
4. **Remote controller** — power both radio nodes and confirm manual control,
   emergency stop and autonomous mode.

Connect and upload to one Arduino at a time. Disconnect motor power while
uploading and confirm that all controllers share the required ground reference.

## Current limitations

This is the preserved firmware of the competition prototype. It demonstrates
the original control logic, but it is not an industrial safety controller.
Notable limitations include blocking delays, hard-coded RFID UIDs and servo
angles, a raw radio packet without validation, and no radio-loss watchdog.
Operate the prototype at low voltage and speed, with an accessible physical
power disconnect.
