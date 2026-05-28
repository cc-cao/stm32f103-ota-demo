#include "stm32f1xx_hal.h"
#include "pwm.h"

void motor_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpio_def = {
        .Pin = GPIO_PIN_6,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_PULLUP,
        .Speed = GPIO_SPEED_LOW,
    };
    HAL_GPIO_Init(GPIOA, &gpio_def);
    pwm_init(720 - 1, 100 - 1);
}

void motor_set_speed(int32_t speed) {
    if (speed >= 0) {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
        speed = -speed;
    }

    pwm_set_compare(TIM_CHANNEL_3, speed);
}
