#include "wdg.h"

IWDG_HandleTypeDef hiwdg;
WWDG_HandleTypeDef hwwdg;

void iwdg_init() {
    __HAL_RCC_LSI_ENABLE();

    hiwdg.Instance = IWDG;
    hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
    hiwdg.Init.Reload = 4096 - 1;

    HAL_IWDG_Init(&hiwdg);
}

void iwdg_feed() {
    HAL_IWDG_Refresh(&hiwdg);
}

void wwdg_init() {
    __HAL_RCC_WWDG_CLK_ENABLE();

    hwwdg.Instance = WWDG;
    hwwdg.Init.Prescaler = WWDG_PRESCALER_8;
    hwwdg.Init.Counter = 0x7f;
    hwwdg.Init.Window = 0x5f;
    hwwdg.Init.EWIMode = WWDG_EWI_DISABLE;
    HAL_WWDG_Init(&hwwdg);
}

void wwdg_feed() {
    HAL_WWDG_Refresh(&hwwdg);
}
