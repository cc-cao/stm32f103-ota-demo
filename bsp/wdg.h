#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

void iwdg_init();
void iwdg_feed();

void wwdg_init();
void wwdg_feed();

#ifdef __cplusplus
}
#endif
