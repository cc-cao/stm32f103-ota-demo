#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

void encoder_init();
int16_t encoder_get();

#ifdef __cplusplus
}
#endif
