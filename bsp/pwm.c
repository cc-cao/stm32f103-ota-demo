#include "stm32f1xx_hal.h"

static TIM_HandleTypeDef htim2;

void pwm_init(uint32_t prescaler, uint32_t period) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    GPIO_InitTypeDef gpio_def = {
        .Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
        .Mode = GPIO_MODE_AF_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_HIGH,
    };
    HAL_GPIO_Init(GPIOA, &gpio_def);

    htim2.Instance = TIM2;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Prescaler = prescaler;
    htim2.Init.Period = period;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_PWM_Init(&htim2);

    TIM_OC_InitTypeDef toc_def = {
        .OCMode = TIM_OCMODE_PWM1,
        .OCPolarity = TIM_OCPOLARITY_HIGH,
        .OCIdleState = TIM_OCIDLESTATE_RESET,
        .Pulse = 0,
    };
    HAL_TIM_PWM_ConfigChannel(&htim2, &toc_def, TIM_CHANNEL_1);
    HAL_TIM_PWM_ConfigChannel(&htim2, &toc_def, TIM_CHANNEL_2);
    HAL_TIM_PWM_ConfigChannel(&htim2, &toc_def, TIM_CHANNEL_3);

    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
}

void pwm_set_compare(uint32_t channel, uint32_t period) {
    __HAL_TIM_SET_COMPARE(&htim2, channel, period);
}
