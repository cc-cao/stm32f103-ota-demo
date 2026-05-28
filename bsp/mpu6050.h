#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef struct {
    int16_t accx;
    int16_t accy;
    int16_t accz;
    int16_t gyrox;
    int16_t gyroy;
    int16_t gyroz;
} MPUData;

void mpu_init();
void mpu_regw(uint8_t addr, uint8_t data);
uint8_t mpu_regr(uint8_t addr);
void mpu_read(MPUData* data);
uint8_t mpu_readid();

#ifdef __cplusplus
}
#endif
