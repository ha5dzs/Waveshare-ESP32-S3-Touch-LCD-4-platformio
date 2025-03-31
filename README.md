# Waveshare ESP32-S3-Touch-LCD-4 PlatformIO

This 'documentation' here is more like a log. I make notes here as I go along, so the order is as the problems arise and I solve them. There are two versions of this board. As far as I can tell, the key difference is the I²C interface of the IO expander.

## Key differences

* Uses PlatformIO and Adruino framework
    * While I prefer RTOS, having access to a ton of decent libraries is a massive plus
    * I prefer PlatformIO to the Arduino IDE.
* Custom header file for pins and system-wide settings

## Hardware

Let's see what intricacies this board has.

### TCA9554 IO Expander

This is on the I²C bus, `A0`, `A1`, `A2` are all pulled to GND, so its base address is `0x20`. The I/O pins are wired up to:

| IO pin | Direction | Where does it go |
|-------|--------|-----------------------|
| P0 | Output | Touch panel reset line |
| P1 | Output | Backlight enable |
| P2 | Output | LCD reset line |
| P3 | Output | SD card Chip Select line |
| P4 | :smile: | Not connected |
| P5 | Output | Beeper thing |
| P6 | Input | Interrupt from the RTC (clock alarm) |
| P7 | ??? | Interrupt pin for the SW6106 power management chip

### ST7701S display

[SPI for mode setting and condfiguration, parallel 16-bit RGB](https://github.com/displaymodule/Examples/blob/master/TFT/ST7701S/TM4.0/390/4inch%20480480%20Initialized%20code-3SPI-16BIT%20RGB.txt) (+Hsync, Vsync, Pixel Clock, Data enable)
There is some ambiguitiy with the display. The code says it's driven in 16-bit mode. The schematic shows that it is wired up for 18-bit mode at the connector, but not all pins are present at the ESP32. The schematic also implies that the ST7701 controller is configured via I²C, whereas in reality, it's SPI. The table does not show where every colour bit is wired to. Luckily I have a multimeter.

| Pin | Where does it go? | Description | 16-bit color bits assignment in the code |
|------------|------------------|---------|----------|
| 1 | Boost converter | Backlight LED's anode | `N/A` |
| 2 & 3 | Boost converter | Backlight LED's cathode | `N/A` |
| 4 | GND | Yes. | `N/A` |
| 5 | 3.3V for the LCD controller | :tv: | `N/A` |
| 6 | TCA9554 IO 3 | LCD Reset | `N/A` |
| 7 & 8 | Nowhere. | :x: | `N/A` |
| 9 | ESP32 IO 1 | SPI MOSI pin. Shared with the SD card too. | `N/A` |
| 10 | ESP32 IO 2 | SPI SCK pin. Shared with the SD card too. | `N/A` |
| 11 | ESP32 IO 42 | CS for the display. | `N/A` |
| 12 | ESP32 IO 41 | Pixel clock | `N/A` |
| 13 | ESP32 IO 40 | Data Enable pin | `N/A` |
| 14 | ESP32 IO 39 | VSync | `N/A` |
| 15 | ESP32 IO 38 | HSync | `N/A` |
| 16 | Nowhere | B0 on schematic, not shown in table | |
| 17 | ESP32 IO 5 | B1 on schematic |`B0` |
| 18 | ESP32 IO 45 | B2 on schematic | `B1` |
| 19 | ESP32 IO 48 | B3 on shematic | `B2` |
| 20 | ESP32 IO 47 | B4 on schmeatic | `B3` |
| 21 | ESP32 IO 21 | B5 on schematic | `B4` |
| 22 | ESP32 IO 14 | G0 on schematic | `G0` |
| 23 | ESP32 IO 13 | G1 on schrmatic | `G1`|
| 24 | ESP32 IO 12 | G2 on schematic | `G2` |
| 25 | ESP32 IO 11 | G3 on schematic | `G3` |
| 26 | ESP32 IO 10 | G4 on schematic | `G4` |
| 27 | ESP32 IO 9 | G5 on schematic | `G5`|
| 28 | Nowhere | R0 on schematic, not shown in table | |
| 29 | ESP32 IO 46 | R1 on schematic | `R0` |
| 30 | ESP32 IO 3 | R2 on schematic | `R1` |
| 31 | ESP32 IO 8 | R3 on schematic | `R2` |
| 32 | ESP32 IO 18 | R4 on schematic | `R3` |
| 33 | ESP32 IO 17 | R5 on schematic | `R4` |
| 34 | GND | Yes. | |
| 35 | ESP32 IO 16 | Touch panel interrupt | `N/A` |
| 36 | ESP32 IO 15 | I²C SDA pin | `N/A` |
| 37 | ESP32 IO 7 | I²C SCL pin | `N/A` |
| 38 | TCA9554 IO 0 | Touch panel reset | `N/A` |
| 39 | 3.3V for the touch panel IC | | `N/A` |
| 40 | GND | Yes. | |

...and this seems to correspond with what's in the code, and with other boards that use this display. The source of the confusion was that they used the 18-bit line names in the schematic, but the bits shift when using 16-bit colour mode for the blue and red colour channels. So what is B[1...5] on the schematic becomes `B[0...4]` int he code.

### GT911 touch panel

Wired to I²C, reset is on IO 0 in the TCA9554 IO expander, interrupt is wired to ESP32 IO 16.







