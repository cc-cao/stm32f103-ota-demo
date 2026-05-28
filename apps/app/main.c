#include "generic.h"
#include "bsp/usart.h"
#include "bsp/led.h"
#include "bsp/wdg.h"
#include "ota.h"
#include "partition.h"

int main(void) {
    // 重定向中断向量表
    SCB->VTOR = APP_A_BASE;
    SystemClock_Config();
    HAL_Init();
    usart1_init();
    led_init();
    ota_init();

    /* 如果是新固件试运行: 跑一段稳定时间后回写 stage=IDLE，否则 IWDG 复位由 bootloader 接管回滚 */
    bool verifying = ota_in_verify_stage();
    uint32_t verify_start = HAL_GetTick();
    bool verified = !verifying;

    iwdg_init();
    HAL_UART_Transmit(&huart1, (uint8_t*)"\r\nhello app new\r\n", 17, HAL_MAX_DELAY);

    for (;;) {
        led_toggle();
        HAL_Delay(200);

        ota_poll();
        iwdg_feed();

        /* 业务跑稳 5 秒就标记为可用版本 */
        if (!verified && (HAL_GetTick() - verify_start) > 5000U) {
            ota_mark_valid();
            verified = true;
            HAL_UART_Transmit(&huart1, (uint8_t*)"app validated\r\n", 15, HAL_MAX_DELAY);
        }
    }
    return 0;
}
