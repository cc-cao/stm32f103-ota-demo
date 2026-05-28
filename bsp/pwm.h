#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

void pwm_init(uint32_t prescaler, uint32_t period);
void pwm_set_compare(uint32_t channel, uint32_t period);

#ifdef __cplusplus
}
#endif
