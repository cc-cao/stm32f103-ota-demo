#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

void motor_init();
void motor_set_speed(int32_t speed);

#ifdef __cplusplus
}
#endif
