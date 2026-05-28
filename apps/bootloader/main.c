#include "generic.h"
#include "bsp/usart.h"
#include "bsp/wdg.h"
#include "bsp/flash.h"
#include "config.h"
#include "common.h"
#include "partition.h"
#include <string.h>

/*
 * Bootloader 启动流程
 * ---------------------------------------------------------------
 *  config.stage 决定走哪条:
 *    PENDING : 新固件已下到 C 槽，校验通过 → A→B 备份 → C→A → stage=VERIFY
 *              → 启 IWDG → 跳 A 试运行 (新 app 必须 mark_valid 把 stage 改回 IDLE)
 *    VERIFY  : 上次试运行没活下来 (IWDG 复位回到 bootloader)，触发回滚:
 *              B→A 还原 → stage=IDLE → 启 IWDG → 跳 A
 *    IDLE/其它: 直接跳 A
 *
 *  注意: 启 IWDG 后整个 bootloader 必须及时跳出去，否则 watchdog 自己会咬到 bootloader。
 *  这里的 IWDG 周期 (~3.2s) 远长于 boot2app 耗时，没问题。
 */

static void copy_slot(uint32_t dst_base, uint32_t src_base, uint32_t size) {
    flash_page_erase(dst_base, size / FLASH_PAGE_SIZE);
    /* 半字写: 一次 2 字节，从源 flash 直接读 */
    for (uint32_t off = 0; off < size; off += 2U) {
        uint16_t hw = *(volatile uint16_t*)(src_base + off);
        flash_write(dst_base + off, &hw, 1U);
    }
}

static void log_str(const char *s) {
    HAL_UART_Transmit(&huart1, (uint8_t*)s, (uint16_t)strlen(s), HAL_MAX_DELAY);
}

static void apply_pending(otadata_t *cfg) {
    log_str("ota: applying pending\r\n");

    /* 校验 C 槽完整性，防 stage=PENDING 但 C 内容损坏 */
    uint32_t c_crc = crc32((const uint8_t*)APP_C_BASE, cfg->image_size);
    if (c_crc != cfg->image_crc) {
        log_str("ota: slot C crc bad, abort\r\n");
        cfg->stage = OTA_IDLE;
        config_save(cfg);
        return;
    }

    /* A → B 备份 */
    copy_slot(APP_B_BASE, APP_A_BASE, APP_A_SIZE);

    /* C → A */
    copy_slot(APP_A_BASE, APP_C_BASE, APP_A_SIZE);

    /* 进入试运行阶段 */
    cfg->stage = OTA_VERIFY;
    config_save(cfg);
    log_str("ota: now VERIFY, jumping\r\n");
}

static void apply_rollback(otadata_t *cfg) {
    log_str("ota: rollback B->A\r\n");

    /* B 槽空 → 没法回滚，只能放弃，把 stage 清掉避免无限自咬 */
    uint32_t b_sp = *(volatile uint32_t*)APP_B_BASE;
    if ((b_sp & 0x2FFE0000U) != 0x20000000U) {
        log_str("ota: slot B empty, give up rollback\r\n");
        cfg->stage = OTA_IDLE;
        config_save(cfg);
        return;
    }

    copy_slot(APP_A_BASE, APP_B_BASE, APP_A_SIZE);

    cfg->stage = OTA_IDLE;
    config_save(cfg);
}

int main(void) {
    SystemClock_Config();
    HAL_Init();
    usart1_init();
    log_str("hello boot\r\n");

    otadata_t cfg = {0};
    bool have_cfg = config_load(&cfg);

    if (have_cfg) {
        switch (cfg.stage) {
        case OTA_PENDING:
            apply_pending(&cfg);
            break;
        case OTA_VERIFY:
            /* 上次试运行没成功 mark_valid，IWDG 复位过来的 */
            apply_rollback(&cfg);
            break;
        default:
            break;
        }
    }

    /* 跳之前先校验 A 槽的 SP 是不是合法的 SRAM 地址。
     * 如果不合法，说明 A 槽没有有效固件，启 IWDG 跳过去会死循环重启 */
    uint32_t a_sp = *(volatile uint32_t*)APP_A_BASE;
    if ((a_sp & 0x2FFE0000U) != 0x20000000U) {
        log_str("ota: slot A invalid, hang\r\n");
        /* 把 stage 清回 IDLE，避免下次启动又走 PENDING/VERIFY 把更多空数据搬过来 */
        cfg.stage = OTA_IDLE;
        config_save(&cfg);
        for (;;) { HAL_Delay(1000); }
    }

    /* IWDG 必须放在 boot2app 之前的最后一步: 启了之后只能由 app 喂 */
    iwdg_init();
    boot2app(APP_A_BASE);

    /* boot2app 不应该返回 */
    for (;;) {
        HAL_Delay(1000);
    }
    return 0;
}
