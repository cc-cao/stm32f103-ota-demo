#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

void ic_init();
uint32_t ic_get_freq();
void pwmi_init();
uint32_t pwmi_get_duty();

#ifdef __cplusplus
}
#endif
