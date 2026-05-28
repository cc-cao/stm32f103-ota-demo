#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

void bee_init();
void bee_on();
void bee_off();
void bee_toggle();

#ifdef __cplusplus
}
#endif
