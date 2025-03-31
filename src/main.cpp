#include "Waveshare_ESP32_S3_Touch_LCD_4.h"
#include <Arduino.h>
#include <TCA9554.h>
#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>


// IO expander.
#if defined V1
// Software-bit-banging I2C on alternate pins. Can't test this, I have a V2 board.
#include <SoftI2C.h>
SoftI2C SoftWire =SoftI2C(IO_EXPANDER_SDA, IO_EXPANDER_SCL); //sda, scl
TCA9554 expander(EXPANDER_ADDRESS, SoftWire);
#else
// V2 board, everything is on the I2C bus
TCA9554 expander(EXPANDER_ADDRESS);
#endif

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
  st7701_type9_init_operations, sizeof(st7701_type9_init_operations)
);

// Touch panel
//TAMC_GT911 touch_panel(SDA, SCL, )

void setup()
{
  // Print out the amount of memory available, see if the PSRAM is visible
  Serial.begin(115200);
  Serial.printf("Available PSRAM: %d KB\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM)>>10);

  // Use the expander to do resets and general signs of life.




}

void loop()
{
  ;
}