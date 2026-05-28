#include "light_sensor.h"

void light_sensor_init() {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitTypeDef gpio_def = {
        .Pin = GPIO_PIN_13,
        .Mode = GPIO_MODE_INPUT,
        .Pull = GPIO_PULLUP,
    };
    HAL_GPIO_Init(GPIOB, &gpio_def);
}

GPIO_PinState light_sensor_read() {
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13);
}
