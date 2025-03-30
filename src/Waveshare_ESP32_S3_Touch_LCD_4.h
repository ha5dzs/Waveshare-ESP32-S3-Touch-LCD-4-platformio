/*
 * Pins and constants for the Waveshare ESP32-S3-Touch-LCD-4 board.
*/

#define V2 // If you have a V1 board, just change this to V1

// I2C
#define SCL 7
#define SDA 15

// The V1 of this board has the IO expander chip's I2C lines wired to different GPIO pinds
#if defined V1
#define IO_EXPANDER_SCL 9
#define IO_EXPANDER_SDA 8
#endif

// Backlight
#define TFT_BL EXIO2

// Touch panel
#define TP_SCL SCL
#define TP_SDA SDA
#define TP_INT 16
#define TP_RST EXIO1

// SPI
#define MOSI 1
#define MISO 4
#define SCK 2

// SD Card
#define SDCARD_MOSI MOSI
#define SDCARD_MISO MISO
#define SDCARD_SCK SCK
// The SD card's Chip Select is on the IO expander.
#define SDCARD_CS EXIO4

// Display.
#define TFT_RESET EXIO3
#define TFT_HS 38
#define TFT_VS 39
#define TFT_PCLK 41
#define TFT_CS 42
#define TFT_DE 40
    // The data lines on the schematic are ambiguous, stuff is missing.
#define TFT_B0_D0
