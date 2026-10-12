# Smart-RFID-lock
simple PCB design for Hack Club Half Life that works as an esp32-c3 controlled RFID lock

## Firmware wiring

The firmware uses the MFRC522 reader over SPI:

| RC522 signal | ESP32-C3 GPIO |
| --- | ---: |
| SCK | 8 |
| MISO | 9 |
| MOSI | 10 |
| SDA/SS | 6 |
| RST | 7 |
| 3.3V | 3.3V |
| GND | GND |

The RC522 should remain configured for SPI. The `SDA` pin on an RC522 SPI
module is the SPI chip-select signal, not an I²C data connection.

Before uploading, replace `AUTHORIZED_UID` in
[`Smart-lock firmware/src/main.cpp`](Smart-lock%20firmware/src/main.cpp) with
the UID of the card that should unlock the relay. The relay is energized for
five seconds after an authorized card is detected.
