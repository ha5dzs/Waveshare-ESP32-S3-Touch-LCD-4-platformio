/*
 * Pins and constants for the Waveshare ESP32-S3-Touch-LCD-4 board.
*/

#define V2 /* If you have a V1 board, just change this to V1 */

// Some defines for the IO expander. To avoid confusion with the GPIO pin numbering, the IO expander's pins have a 'P' prefix.
#define EXPANDER_ADDRESS 0x20
#define P0 0 /* Touch panel reset */
#define P1 1 /* Backlight enable */
#define P2 2 /* TFT Reset */
#define P3 3 /* SD card chip select */
#define P4 4 /*  This is not wired up to anything */
#define P5 5 /* Beep */
#define P6 6 /* RTC alarm interrupt */
#define P7 7 /* Power management interrupt */


// The V1 of this board has the IO expander chip's I2C lines wired to different GPIO pinds
#if defined V1
#define IO_EXPANDER_SCL 9
#define IO_EXPANDER_SDA 8
#endif

//I2C
#define I2C_SCL 7
#define I2C_SDA 15
#define I2C_SPEED 50000 /* Keep the I2C slow, because we don't know how long the wiring will be.*/

// Backlight
#define TFT_BL P1

// SD Card (in SPI mode)
// The SD card's Chip Select is on the IO expander.
#define SDCARD_CS P3
#define SDCARD_MOSI 1
#define SDCARD_MISO 4

// Display.
#define TFT_SCK 2
#define TFT_SDA 1
// Control pins
#define TFT_RESET P2
#define TFT_HS 38
#define TFT_VS 39
#define TFT_PCLK 41
#define TFT_CS 42
#define TFT_DE 40
// Colour bits, see README.md for this
// Blue
#define TFT_B0 5
#define TFT_B1 45
#define TFT_B2 48
#define TFT_B3 47
#define TFT_B4 21
// Green
#define TFT_G0 14
#define TFT_G1 13
#define TFT_G2 12
#define TFT_G3 11
#define TFT_G4 10
#define TFT_G5 9
// Red
#define TFT_R0 46
#define TFT_R1 3
#define TFT_R2 8
#define TFT_R3 18
#define TFT_R4 17

#define TFT_DATA_SPEED 13000000
#define TFT_WIDTH 480
#define TFT_HEIGHT 480
#define ROTATION 2

#define TFT_16BIT_BGR_FORMAT true
#define TFT_HSYNC_POLARITY 1
#define TFT_HSYNC_FRONT_PORCH 10
#define TFT_HSYNC_PULSE_WIDTH 8
#define TFT_HSYNC_BACK_PORCH 50

#define TFT_VSYNC_POLARITY 1
#define TFT_VSYNC_FRONT_PORCH 10
#define TFT_VSYNC_PULSE_WIDTH 8
#define TFT_VSYNC_BACK_PORCH 20

#define TFT_PCLK_ACTIVE_NEG 0
#define TFT_USE_BIG_ENDIAN false

// Arduino GFX-specific stuff.
#define TFT_AUTO_FLUSH false

// Touch panel
#define TP_INT 16
#define TP_RESET P0

// This is for use with the TAMC_GT911 library: make sure that the screen rotates together with the touch coordinates.
#if defined(ROTATION) && ROTATION == 0
#define TAMC_GT911_ROTATION 1
#elif defined(ROTATION) && ROTATION == 1
#define TAMC_GT911_ROTATION 2
#elif defined(ROTATION) && ROTATION == 2
#define TAMC_GT911_ROTATION 3
#elif defined(ROTATION) && ROTATION == 3
#define TAMC_GT911_ROTATION 0
#endif

// Beeper thing
#define BEEPER P5

// RTC interrupt pin
#define RTC_INT P6

// Power-management interrupt
#define PM_INT P7

// CAN bus
#define CAN_TX 6
#define CAN_RX 0
#define CAN_POLLING_RATE_MS 1000
#define CAN_TRANSMIT_RATE_MS 1000

// RS485, which is wired to UART0 but with a weird circuit
#define RS485_BITRATE 115200 /* in bits per second. */
#define RS485_DIRECTION 44 /* Physical pin 36: High: Input, Low: Output */
#define RS485_RECEIVE_PIN 43 /* Physical pin 37 */
