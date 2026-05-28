#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

void w25q80_init();
void w25q80_readid(uint8_t* mid, uint16_t* did);
void w25q80_write_enable();
void w25q80_wait_busy();
void w25q80_sector_erase(uint32_t addr);
void w25q80_page_program(uint32_t addr, uint8_t* data, uint16_t size);
void w25q80_read_data(uint32_t addr, uint8_t* data, uint32_t size);

#ifdef __cplusplus
}
#endif
