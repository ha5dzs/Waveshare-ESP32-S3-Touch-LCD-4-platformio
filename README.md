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

Wired to I²C, reset is on IO 0 in the TCA9554 IO expander, interrupt is wired to ESP32 IO 16. For now, I won't use it to attach interrupt, because LVGL regularly checks the input devices anyway. But, according to the datasheet, the interrupt pin is used in the reset procedure as per  [TAMC_GT911::reset()](https://github.com/TAMCTec/gt911-arduino/blob/main/TAMC_GT911.cpp), the inerrupt pin has to be driven low when the reset is pulled low, and depending on timing, the address of the chip can be selected. If within 5 milliseconds of the reset pin going high, the interrupt pin is pulled high too, then the controller will change to the alternative address, which it will keep until the next reset procedure.

In this booard, achieving this can be a bit tricky, because the GT911 reset pin is though the IO expander `P0`, but the interrupt pin is wired up directly to the ESP32 IO 16. So for now, this is kept on default.

For future reference, in case the alternate address is needed, the initialisation should be moved to `tca_expander_reset_dance()`, and timing should be verified with an oscilloscope.

### The RTC situation PCF85063A RTC module and the internal RTC in the ESP32

The ESP32 has an internal RTC, but has no backup battery. In an ideal world, where there is always network connectivity, one could set up a local time server and sync time during bootup. But the world is cruel. So, to compensate for this, Waveshare people added an external RTC, which is connected to the I²C bus, and perhaps more importantly, has the option to connect an external battery. So when the board is powered off, the time settings are preserved.

During bootup, the code sets the ESP RTC to the PCF85036A RTC. And later-on, if there is a network connection, the local RTC gets synced using NTP, and the PCF85036A is set accordingly. Another caveat is that the [Soldered-PCF85063A-RTC-Module-Arduino-Library](https://github.com/SolderedElectronics/Soldered-PCF85063A-RTC-Module-Arduino-Library/tree/main) does not support time zones, but [ESP32Time](https://github.com/fbiego/ESP32Time) does.

## SW6106 power management IC

There is no dedicated Arduino library for this, but [someone asked this on StackExchange](https://arduino.stackexchange.com/questions/70420/reading-i2c-data-from-sw6106-register) and [someone else got a nice collection of datasheets and register descriptions](https://archive.org/details/sw-6106-schematic-release-sch-006-v-2.2) here. This chip supports power delivery and fast charging. It is also possible to set the interrupt pin according to a high number of conditions. As this board can be powered externally from the interface connector and has its own buck converter and I don't plan to use a battery at all, I think I'll leave this alone for now.

## CAN bus

The board has a CAN bus transceiver, and the `CANH` and `CANL` lines are wired to the interface connector. Through the IO multiplexer in the ESP32, it is possible to receive and transmit CAN frames using its internal dedicated hardware. For some reason, they don't call it CAN (Control Area Network), but they call it [TWAI (Two-Wire Automotive Interface)](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/twai.html). Due to a large number of variants, its code is moved to a separate file.

## Software

## Environment setup

`platformio.ini` is used to rename the environment and configure the external memory.

```ini
[env:Waveshare_ESP32_S3_Touch_LCD_4]
platform = espressif32
framework = arduino
board = esp32-s3-devkitm-1
; A couple of overrides.
board_build.arduino.memory_type = qio_opi
board_build.flash_mode = qio
board_build.psram_type = opi
board_upload.flash_size = 16MB
board_upload.maximum_size = 16777216
monitor_speed = 115200
build_flags =
    -DARDUINO_USB_MODE=1 ; Serial port please?
    -DARDUINO_USB_CDC_ON_BOOT=1  ; We need CDC for the serial port stuff
    -DBOARD_HAS_PSRAM
    -DLV_CONF_INCLUDE_SIMPLE
    -DLV_USE_DEMO_WIDGETS
lib_deps =
    https://github.com/moononournation/Arduino_GFX
    lvgl/lvgl@^8.4.0
    https://github.com/yasir-shahzad/SoftI2C.git
    https://github.com/RobTillaart/TCA9554.git
    https://github.com/tamctec/gt911-arduino
    https://github.com/SolderedElectronics/Soldered-PCF85063A-RTC-Module-Arduino-Library.git
    https://github.com/fbiego/ESP32Time.git
```

This board has the USB wired directly from the MCU, so `-DARDUINO_USB_MODE=1` and `-DARDUINO_USB_CDC_ON_BOOT=1` are used.

**IMPORTANT things that make the board difficult to work with at first:**

* The board comes with the LVGL widgets demo, which runs slowly
    * USB CDC is not enabled, so one must to the `Reset` -> `Reset + Boot` -> `Boot` combo in order to upload new code
* The SW6101 chip is somehow misconfigured
    * It tries to request USB PD when plugged in a computer? At least something seems to touch VBUS
    * When pressing `BAT_PWR`, it just flashes once when powered from a computer
    * When plugged into a smart charger that supports USB PD, it doesn't do anything
    * The board only starts when:
        * The USB cable is plugged to a simple 5V charger
        * External power supply via the interface connector

When using the external power supply, I noticed that I got rapid connection and disconnection sounds from my computer when plugging in. This is because the GND on the interface connector and the outer shield of the USB-C connector are galvanically connected to the GND of the USB line. While this is not 'wrong' per se, the USB connection fails due to a ground loop from my external power supply and the computer. To upload the first version of this code, I had to use the external power supply, and connect the USB to a battery-powered computer.

Afterwards, the power consumption is low enough to be powered from the USB port, and since the SW6106 device is not configured, it no longer interferes with the USB poweer.

BUT: as long as `-DARDUINO_USB_MODE=1` and `-DARDUINO_USB_CDC_ON_BOOT=1` are enabled, it seems that it won't boot until valid USB connection is established to a host computer. So when using this in an external project, when going into production, these two flags should be cleared.

### Low-level display access

The St7701 display is configured via sotware SPI, and then instructed to work in 16-bit parallel load RGB.

```C
// Software SPI to configure the display.
Arduino_DataBus *sw_spi_bus = new Arduino_SWSPI(GFX_NOT_DEFINED /* DC pin */, TFT_CS /* TFT Chip Select */, TFT_SCK /* SPI clock */, TFT_SDA /* MOSI */, GFX_NOT_DEFINED /* MISO */);

// Display hardware definition
Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
  TFT_DE /* Data Enable */, TFT_VS /* Vertical sync */, TFT_HS /* Horizontal sync */, TFT_PCLK /* Pixel clock */,
  TFT_R0, TFT_R1, TFT_R2, TFT_R3, TFT_R4 /* Red channel*/,
  TFT_G0, TFT_G1, TFT_G2, TFT_G3, TFT_G4, TFT_G5 /* Green channel*/,
  TFT_B0, TFT_B1, TFT_B2, TFT_B3, TFT_B4 /* Blue channel*/,
  TFT_HSYNC_POLARITY, TFT_HSYNC_FRONT_PORCH, TFT_HSYNC_PULSE_WIDTH, TFT_HSYNC_BACK_PORCH /* Horizontal sync settings, times are in ns, apparently */,
  TFT_VSYNC_POLARITY, TFT_VSYNC_FRONT_PORCH, TFT_VSYNC_PULSE_WIDTH, TFT_VSYNC_BACK_PORCH /* Vertical sync settings, similar to above */,
  TFT_PCLK_ACTIVE_NEG /* Falling edge? Active low? */, TFT_DATA_SPEED, TFT_USE_BIG_ENDIAN
);

// Low-level display object
Arduino_RGB_Display *tft = new Arduino_RGB_Display(
  TFT_WIDTH, TFT_HEIGHT, rgbpanel, ROTATION, TFT_AUTO_FLUSH /* Auto flush is false, because it is done from lvgl.*/,
  sw_spi_bus, GFX_NOT_DEFINED /* Resetting the panel is done during the reset dance */,
  st7701_type1_init_operations, sizeof(st7701_type1_init_operations)
);
```

To check successful initialisation, a test pattern is displayed using the following code in `setup()`:

```C
for (uint16_t x_coord = 0; x_coord < TFT_WIDTH; x_coord++)
  {
    for (uint16_t y_coord = 0; y_coord < TFT_HEIGHT; y_coord++)
    {
      // X, Y, colour. In this case, 16 bits.
      tft -> writePixel(x_coord, y_coord, tft->color565( x_coord<<1, (x_coord + y_coord)<<2, y_coord<<1));
    }
  }
  tft->flush();
```

### LVGL

LVGL 8.4 is used here, becuase I found a possible bug [in another project that use the same display](https://github.com/ha5dzs/Guition-ESP32-4848S040-platformio).



