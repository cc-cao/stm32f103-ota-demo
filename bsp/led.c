#include "stm32f1xx_hal.h"

void led_init() {
    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef gpio_def = {
        .Pin = GPIO_PIN_13,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_LOW,
    };
    HAL_GPIO_Init(GPIOC, &gpio_def);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
}

void led_toggle() {
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
}

void led_set(int state) {
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, state ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

void led_a12_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    GPIO_InitTypeDef gpio_def = {
        .Pin = GPIO_PIN_1 | GPIO_PIN_2,
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_LOW,
    };
    HAL_GPIO_Init(GPIOA, &gpio_def);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1 | GPIO_PIN_2, GPIO_PIN_SET);
}

void led_a1_on() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
}

void led_a1_off() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
}

void led_a1_toggle() {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_1);
}

void led_a2_on() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_RESET);
}

void led_a2_off() {
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2, GPIO_PIN_SET);
}

void led_a2_toggle() {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_2);
}
