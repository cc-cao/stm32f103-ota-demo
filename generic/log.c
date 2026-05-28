#include "log.h"

#ifdef USE_PRINT
int _read(int file, char* ptr, int len) {
    UNUSED(file);
    if (HAL_UART_Receive(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY) != HAL_OK)
        return 0;
    return len;
}

int _write(int file, char* ptr, int len) {
    UNUSED(file);
    if (HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY) != HAL_OK)
        return 0;
    return len;
}
#endif
