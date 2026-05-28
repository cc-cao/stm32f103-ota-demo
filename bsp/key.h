#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

void key_init();
uint8_t key_getnum();

#ifdef __cplusplus
}
#endif

