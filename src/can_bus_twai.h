// From: https://community.platformio.org/t/files-in-lib-compile-but-linker-cant-find-them-resolved/10489
#ifdef __cplusplus
extern "C" {
#endif
#include "Waveshare_ESP32_S3_Touch_LCD_4.h"
#include <driver/twai.h>

void can_bus_init(void);
void can_bus_receive(void);
void can_bus_transmit(void);


#ifdef __cplusplus
}
#endif