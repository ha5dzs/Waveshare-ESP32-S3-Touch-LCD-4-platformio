/*
 * The first thing to do here is to initialise the expander, and set up the correct IO pins.
 * Make a small beep, enable the backlight, etc.
*/
#include "Waveshare_ESP32_S3_Touch_LCD_4.h"
#include <Arduino.h>
#include <TCA9554.h>

// This is declared in the main code.
extern TCA9554 expander;

void tca_expander_reset_dance(void)
{
  Wire.begin();
  expander.begin();
  // To test this, make a small beep.
  expander.pinMode1(BEEPER, OUTPUT);
  expander.write1(BEEPER, LOW);
  expander.write1(BEEPER, HIGH);
  delay(100);
  expander.write1(BEEPER, LOW);

  // Reset the display module
  expander.pinMode1(TFT_RESET, OUTPUT);
  expander.write1(TFT_RESET, LOW); // reset is active low, and must last for at least 10 microseconds.
  delay(1); // So wait for a millisecond
  expander.write1(TFT_RESET, HIGH);

  // Reset the touch panel controller
  expander.pinMode1(TP_RESET, OUTPUT);
  expander.write1(TP_RESET, LOW); // reset is active low, must last for at least 100 microseconds.
  delay(1); // So, again, wait a millisecond
  expander.write1(TP_RESET, HIGH);

  // SC Card chip select
  expander.pinMode1(SDCARD_CS, OUTPUT);
  expander.write1(SDCARD_CS, HIGH); // Disable the SD card for now, becuase we need to initialise the display first.


  // Turn on backlight
  expander.pinMode1(TFT_BL, OUTPUT);
  expander.write1(TFT_BL, HIGH);

  // RTC Interrupt
  expander.pinMode1(RTC_INT, INPUT);

  // PM interrupt
  expander.pinMode1(PM_INT, INPUT);
}
