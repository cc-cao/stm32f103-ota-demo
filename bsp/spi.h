#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

void spi_init();
void spi_start();
void spi_stop();
uint8_t spi_swap_byte(uint8_t byte);

#ifdef __cplusplus
}
#endif
