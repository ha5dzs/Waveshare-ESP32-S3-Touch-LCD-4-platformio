#include "Waveshare_ESP32_S3_Touch_LCD_4.h"
#include "tca_expander_reset_dance.h"
#include <Arduino.h>
#include <TCA9554.h>
#include <Arduino_GFX_Library.h>
#include <TAMC_GT911.h>
#include <PCF85063A-SOLDERED.h> // RTC on board, with a battery backup
#include <ESP32time.h> // Internal RTC on the ESP32, no backup battery
#include <driver/twai.h>
#include "can_bus_twai.h"
#include <lvgl.h>
#include <Ticker.h>
#include "keyboard_example_scene.h"
//#include <SoftwareSerial.h>
#include <HardwareSerial.h>
#define TICKER_MS 5

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

// Software serial for the RS-485 interface.
//EspSoftwareSerial::UART RS485;
HardwareSerial RS485(1); // Use UART1.

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

// Touch panel. For now, the interrupt pin is ignored in the code, but it is used for hardware initialisation.
TAMC_GT911 touch_panel(I2C_SDA, I2C_SCL, TP_INT /*Touch panel ionterrupt*/, -1 /* Reset pin, do it separately*/, TFT_WIDTH, TFT_HEIGHT );

// Ticker, for LVGL.
Ticker ticker;

// Real-time clock.
PCF85063A external_rtc;
ESP32Time internal_rtc(0); // No timezone defined. specify offset in seconds here in required.

// Can-bus stuff. Look at can_bus_twai.*
twai_message_t can_bus_message_to_send;
twai_message_t can_bus_message_received;

/*
 * LVGL-specific stuff
*/

// Main pointers
static lv_disp_draw_buf_t draw_buffer;
static lv_color_t *frame_buffer;
static lv_disp_drv_t display_driver;

// Display updater function
void my_disp_flush(lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);

    tft->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    tft->flush(); // Do the actual flushing

    lv_disp_flush_ready(disp);
}

// This function is executed every LVGL_TICKER_MS milliseconds.
void ticker_call_function(void)
{
  lv_tick_inc(TICKER_MS);
  lv_task_handler();
}


// Touch panel callback function. LVGL 8.4.0 does not support multitouch.
void my_input_read(lv_indev_drv_t *indev_driver, lv_indev_data_t *data)
{
  touch_panel.read();
  {
    if (touch_panel.isTouched)
    {
      data->state = LV_INDEV_STATE_PRESSED;
      // Since no multitouch, get the first point.
      data->point.x = touch_panel.points[0].x;
      data->point.y = touch_panel.points[0].y;
    }
    else
    {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  }
}

void setup()
{
  Serial.begin(115200); // Initialise the USB CDC UART.

  // The RS485 stuff needs a bit of hacking
  //pinMode(RS485_DIRECTION, OUTPUT);
  //digitalWrite(RS485_DIRECTION, HIGH); // Set the transceiver to receive
  //pinMode(RS485_RECEIVE_PIN, INPUT);
  //RS485.begin(RS485_BITRATE, EspSoftwareSerial::SWSERIAL_8N1, RS485_RECEIVE_PIN); // Softwareserial

  pinMode(RS485_DIRECTION, OUTPUT);
  digitalWrite(RS485_DIRECTION, HIGH); // Set the transceiver to receive
  RS485.begin(RS485_BITRATE, SERIAL_8N1, RS485_RECEIVE_PIN, -1 /* No TX pin*/);

  // Start up the I2C hardware and reset peripherals using the IO expander.
  tca_expander_reset_dance();

  // Ticker
  ticker.attach_ms(TICKER_MS, ticker_call_function);

  // External Real-time clock
  external_rtc.begin(); // This is in local time
  if(external_rtc.getYear() < 2025)
  {
    // If we got here, we lost time, so for now, put a dummy time on.
    external_rtc.setTime(13, 55, 00); // 24H mode, ex. 6:54:00
    external_rtc.setDate(2, 1, 4, 2025); // 0 for Sunday, ex. Saturday, 16.5.2020.

  }
  // During bootup, set the internal RTC to the external RTC. Note how the arguments are backwards
  internal_rtc.setTime(external_rtc.getSecond(), external_rtc.getMinute(), external_rtc.getHour(), external_rtc.getDay(), external_rtc.getMonth(), external_rtc.getYear());


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
  touch_panel.reset(); // This toggles the interrupt pin as per the datasheet
  touch_panel.begin();
  touch_panel.setRotation(TAMC_GT911_ROTATION);
  touch_panel.setResolution(TFT_WIDTH, TFT_HEIGHT);

  // LVGL
  lv_init(); // Start the dance

  // Initialise an entire frame's buffer in the SPI RAM
  frame_buffer = (lv_color_t *)heap_caps_malloc(sizeof(lv_color_t) * TFT_WIDTH * TFT_HEIGHT, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

  // If the PSRAM is not initialised, this should fail. 480x480x2=460800 -> 450 kB
  if(frame_buffer == NULL)
  {
    Serial.println("Unable to allocate memory for the frame buffer. Do you have enough PSRAM?\n");
    while(1);
  }

  // Initialise draw buffer, and assign it to the frame buffer.
  lv_disp_draw_buf_init(&draw_buffer, frame_buffer, NULL, TFT_WIDTH * TFT_HEIGHT);

  // Initialise the display driver, and set some basic details.
  lv_disp_drv_init(&display_driver);
  display_driver.hor_res = TFT_WIDTH;
  display_driver.ver_res = TFT_HEIGHT;
  display_driver.flush_cb = my_disp_flush; // Assign callback for display update
  display_driver.full_refresh = 0; // Always redraw the entire screen. This makes it slower
  display_driver.draw_buf = &draw_buffer; // The memory address where the draw buffer begins

  // Finally, register this display
  lv_disp_drv_register(&display_driver);


  // Initialise the touch panel driver
  static lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);
  indev_drv.type = LV_INDEV_TYPE_POINTER; // No multitouch :()
  indev_drv.read_cb = my_input_read; // This is where we read the touch controller
  lv_indev_drv_register(&indev_drv);


  // Print something
  lv_obj_t *label = lv_label_create( lv_scr_act() );
  lv_label_set_text( label, "LVGL V" GFX_STR(LVGL_VERSION_MAJOR) "." GFX_STR(LVGL_VERSION_MINOR) "." GFX_STR(LVGL_VERSION_PATCH));
  lv_obj_align( label, LV_ALIGN_CENTER, 0, -20 );



  // Call the keyboard sample scene
  lv_example_keyboard_1();


  // Print out the amount of memory available, see if the PSRAM is visible
  Serial.printf("Available PSRAM: %d KB\n", heap_caps_get_free_size(MALLOC_CAP_SPIRAM)>>10);
  // Verify operation
  Serial.printf("PCF85063A RTC says it's %d/%d/%d %d:%d:%d\n", external_rtc.getYear(), external_rtc.getMonth(), external_rtc.getDay(), external_rtc.getHour(), external_rtc.getMinute(), external_rtc.getSecond());
  Serial.print("Internal RTC says: ");
  Serial.println(internal_rtc.getTime("%A, %B %d %Y %H:%M:%S"));
  Serial.print("Current Unix time is: ");
  Serial.println(internal_rtc.getEpoch());

}

void loop()
{
  /*
  // Send touch information via the UART
  touch_panel.read();
  if (touch_panel.isTouched){
    for (int i=0; i<touch_panel.touches; i++){
      Serial0.print("Touch ");Serial0.print(i+1);Serial0.print(": ");;
      Serial0.print("  x: ");Serial0.print(touch_panel.points[i].x);
      Serial0.print("  y: ");Serial0.print(touch_panel.points[i].y);
      Serial0.print("  size: ");Serial0.println(touch_panel.points[i].size);
      Serial0.println(' ');
    }
  }
  */

  // Read from the RS485 port, and spit back data over the CDC serial port.
  while(RS485.available() > 0)
  {
    Serial.write(RS485.read());
    yield(); // Whoa.
  }



}