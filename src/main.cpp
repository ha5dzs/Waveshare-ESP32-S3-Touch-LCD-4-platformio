#include "Waveshare_ESP32_S3_Touch_LCD_4.h"
#include "tca_expander_reset_dance.h"
#include <Arduino.h>
#include <TCA9554.h>
#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>
#include "HWCDC.h"

HWCDC USBSerial;

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
  st7701_type1_init_operations, sizeof(st7701_type1_init_operations)
);

// Touch panel
TAMC_GT911 touch_panel(I2C_SDA, I2C_SCL, -1 /*Touch panel ionterrupt*/, -1 /* Reset pin, do it separately*/, TFT_WIDTH, TFT_HEIGHT );



void setup()
{
  // Start up the I2C hardware and reset peripherals using the IO expander.
  tca_expander_reset_dance();



  // Display hardware
  tft->begin();
  // Test: Throw some pixels out to check that the low-level stuff works.
  tft->flush();
  for (uint16_t x_coord = 0; x_coord < TFT_WIDTH; x_coord++)
  {
    for (uint16_t y_coord = 0; y_coord < TFT_HEIGHT; y_coord++)
    {
      // X, Y, colour. In this case, 16 bits.
      tft -> writePixel(x_coord, y_coord, tft->color565( x_coord<<1, (x_coord + y_coord)<<2, y_coord<<1));
    }
  }
  tft->flush();

  // Touch panel
  pinMode(TP_INT, INPUT);
  touch_panel.begin();
  touch_panel.setResolution(TFT_WIDTH, TFT_HEIGHT);


  // Print out the amount of memory available, see if the PSRAM is visible
  Serial.begin(115200);
  Serial.printf("Available PSRAM: %d KB\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM)>>10);
}

void loop()
{
  touch_panel.read();
  if (touch_panel.isTouched){
    for (int i=0; i<touch_panel.touches; i++){
      tft->setCursor(0, i*20);
      tft->print("Touch ");tft->print(i+1);tft->print(": ");;
      tft->print("  x: ");tft->print(touch_panel.points[i].x);
      tft->print("  y: ");tft->print(touch_panel.points[i].y);
      tft->print("  size: ");tft->println(touch_panel.points[i].size);

    }
  }
}