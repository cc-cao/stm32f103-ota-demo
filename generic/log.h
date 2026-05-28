#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <inttypes.h>
#include <string.h>
#include "stm32f1xx_hal.h"
#include "bsp/usart.h"

#if defined(__FILE_NAME__)
#define LOG_FILE_NAME __FILE_NAME__
#else
#define LOG_FILE_NAME (strrchr("/" __FILE__, '/') + 1)
#endif

#ifdef USE_PRINT
#define log_init usart1_init
#define log(fmt, ...) printf("%06" PRIu32 " %s(%d): " fmt "\n", HAL_GetTick(), LOG_FILE_NAME, __LINE__, ##__VA_ARGS__)
#else
#define log_init()
#define log(fmt, ...)
#endif

#ifdef __cplusplus
}
#endif
