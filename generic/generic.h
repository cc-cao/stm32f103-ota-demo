#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include "utypes.h"
#include "log.h"

void SystemClock_Config(void);
void Error_Handler();
void delay_us(uint16_t us);

#ifdef __cplusplus
}
#endif
