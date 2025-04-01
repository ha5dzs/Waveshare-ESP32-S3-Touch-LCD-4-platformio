#include "Waveshare_ESP32_S3_Touch_LCD_4.h"
#include <driver/twai.h>

// These are defined in main.cpp
extern twai_message_t can_bus_message_to_send;
extern twai_message_t can_bus_message_received;

// This tells the compiler that they are in a different source file
extern "C" void can_bus_init(void);
extern "C" void can_bus_receive(void);
extern "C" void can_bus_transmit(void);
