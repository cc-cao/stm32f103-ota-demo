#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

void flash_read(uint32_t addr, uint8_t* data, uint32_t size);
void flash_write(uint32_t addr, uint16_t* data, uint32_t size);
void flash_page_erase(uint32_t addr, uint16_t pages_number);

#ifdef __cplusplus
}
#endif
