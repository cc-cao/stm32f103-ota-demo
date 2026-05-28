#include "encoder.h"

static TIM_HandleTypeDef htim3;

void encoder_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    GPIO_InitTypeDef gpio_def = {
        .Pin = GPIO_PIN_6 | GPIO_PIN_7,
        .Mode = GPIO_MODE_AF_INPUT,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_HIGH,
    };
    HAL_GPIO_Init(GPIOA, &gpio_def);

    htim3.Instance = TIM3;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Prescaler = 1 - 1;
    htim3.Init.Period = 65536 - 1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    HAL_TIM_IC_Init(&htim3);

    TIM_Encoder_InitTypeDef enc_def = {
        .EncoderMode = TIM_ENCODERMODE_TI12,
        .IC1Polarity = TIM_ICPOLARITY_RISING,
        .IC1Prescaler = TIM_ICPSC_DIV1,
        .IC1Selection = TIM_ICSELECTION_DIRECTTI,
        .IC2Polarity = TIM_ICPOLARITY_RISING,
        .IC2Prescaler = TIM_ICPSC_DIV1,
        .IC2Selection = TIM_ICSELECTION_DIRECTTI,
    };
    HAL_TIM_Encoder_Init(&htim3, &enc_def);

    __HAL_TIM_SET_COUNTER(&htim3, 0);

    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_2);
}

int16_t encoder_get() {
    int16_t cnt = (int16_t)__HAL_TIM_GET_COUNTER(&htim3);
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    return cnt;
}
