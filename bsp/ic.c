#include "ic.h"

static TIM_HandleTypeDef htim3;

void ic_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    GPIO_InitTypeDef gpio_def = {
        .Pin = GPIO_PIN_6,
        .Mode = GPIO_MODE_AF_INPUT,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_HIGH,
    };
    HAL_GPIO_Init(GPIOA, &gpio_def);

    htim3.Instance = TIM3;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Prescaler = 72 - 1;
    htim3.Init.Period = 65535 - 1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_IC_Init(&htim3);

    TIM_IC_InitTypeDef ic_def = {
        .ICFilter = 0x00,
        .ICPolarity = TIM_ICPOLARITY_RISING,
        .ICPrescaler = TIM_ICPSC_DIV1,
        .ICSelection = TIM_ICSELECTION_DIRECTTI,
    };
    HAL_TIM_IC_ConfigChannel(&htim3, &ic_def, TIM_CHANNEL_1);

    TIM_SlaveConfigTypeDef slave_config = {
        .InputTrigger = TIM_TS_TI1FP1,
        .SlaveMode = TIM_SLAVEMODE_RESET,
        .TriggerPrescaler = TIM_ICPSC_DIV1,
        .TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING,
    };

    HAL_TIM_SlaveConfigSynchro(&htim3, &slave_config);

    HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_1);
}

uint32_t ic_get_freq() {
    return 1000000 / (__HAL_TIM_GetCompare(&htim3, TIM_CHANNEL_1) + 1);
}

void pwmi_init() {
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_TIM3_CLK_ENABLE();

    GPIO_InitTypeDef gpio_def = {
        .Pin = GPIO_PIN_6,
        .Mode = GPIO_MODE_AF_INPUT,
        .Pull = GPIO_PULLDOWN,
        .Speed = GPIO_SPEED_HIGH,
    };
    HAL_GPIO_Init(GPIOA, &gpio_def);

    htim3.Instance = TIM3;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Prescaler = 72 - 1;
    htim3.Init.Period = 65535 - 1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_IC_Init(&htim3);

    TIM_IC_InitTypeDef ic_def = {
        .ICFilter = 0x00,
        .ICPolarity = TIM_ICPOLARITY_RISING,
        .ICPrescaler = TIM_ICPSC_DIV1,
        .ICSelection = TIM_ICSELECTION_DIRECTTI,
    };
    HAL_TIM_IC_ConfigChannel(&htim3, &ic_def, TIM_CHANNEL_1);
    
    ic_def.ICPolarity = TIM_ICPOLARITY_FALLING;
    ic_def.ICSelection = TIM_ICSELECTION_INDIRECTTI;
    HAL_TIM_IC_ConfigChannel(&htim3, &ic_def, TIM_CHANNEL_2);

    TIM_SlaveConfigTypeDef slave_config = {
        .InputTrigger = TIM_TS_TI1FP1,
        .SlaveMode = TIM_SLAVEMODE_RESET,
        .TriggerPrescaler = TIM_ICPSC_DIV1,
        .TriggerPolarity = TIM_INPUTCHANNELPOLARITY_RISING,
    };

    HAL_TIM_SlaveConfigSynchro(&htim3, &slave_config);

    HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_1);
    HAL_TIM_IC_Start(&htim3, TIM_CHANNEL_2);
}

uint32_t pwmi_get_duty() {
    uint32_t ccr1 = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_1) + 1;
    uint32_t ccr2 = __HAL_TIM_GET_COMPARE(&htim3, TIM_CHANNEL_2) + 1;
    return ccr2 * 100 / ccr1;
}
