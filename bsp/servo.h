#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

void servo_init();
void servo_set_angle(float angle);

#ifdef __cplusplus
}
#endif
