// From: https://community.platformio.org/t/files-in-lib-compile-but-linker-cant-find-them-resolved/10489
#ifdef __cplusplus
extern "C" {
#endif

#include <lvgl.h>
static void ta_event_cb(lv_event_t * e);
void lv_example_keyboard_1(void);

#ifdef __cplusplus
}
#endif