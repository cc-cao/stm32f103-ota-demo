#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

void light_sensor_init();
GPIO_PinState light_sensor_read();

#ifdef __cplusplus
}
#endif
