#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void led_init();
void led_toggle();
void led_set(int state);
void led_a12_init();
void led_a1_on();
void led_a1_off();
void led_a1_toggle();
void led_a2_on();
void led_a2_off();
void led_a2_toggle();

#ifdef __cplusplus
}
#endif
