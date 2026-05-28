#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

extern uint16_t adc_dma[2];

void adc_init();
uint32_t adc_read(uint32_t channel);

#ifdef __cplusplus
}
#endif
