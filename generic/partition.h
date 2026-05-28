#pragma once

/*
 * STM32F103C8 Flash 分区表 (A/B/C 三槽方案)
 *
 * 物理参数:
 *   Flash 起始: 0x08000000
 *   Flash 总量: 64 KB
 *   Page 大小:  1 KB
 *
 * 布局:
 *   +------------------+ 0x08000000
 *   | Bootloader  14KB |
 *   +------------------+ 0x08003800
 *   | otadata      2KB |  ← 双副本 (2 × 1KB sector)
 *   +------------------+ 0x08004000
 *   | Slot A      16KB |  ← 当前运行槽
 *   +------------------+ 0x08008000
 *   | Slot B      16KB |  ← 回滚备份（上个稳定版本）
 *   +------------------+ 0x0800C000
 *   | Slot C      16KB |  ← OTA 下载 staging
 *   +------------------+ 0x08010000
 *
 * 升级流程:
 *   1. App 把新固件下载到 C，校验
 *   2. 重启进 bootloader: A → B (备份当前运行版)
 *   3. C → A
 *   4. 跳 A 运行新版
 *   5. 试运行不通过 → B → A 回滚
 */

#define FLASH_BASE_ADDR     0x08000000U
#define FLASH_TOTAL_SIZE    (64U * 1024U)
#ifndef FLASH_PAGE_SIZE
#define FLASH_PAGE_SIZE     (1U * 1024U)
#endif

/* Bootloader */
#define BOOT_BASE           0x08000000U
#define BOOT_SIZE           (14U * 1024U)

/* otadata: 状态机 + 启动选择，双副本防止写半截掉电 */
#define OTADATA_BASE        0x08003800U
#define OTADATA_SIZE        (2U * 1024U)
#define OTADATA_COPY0_BASE  (OTADATA_BASE)
#define OTADATA_COPY1_BASE  (OTADATA_BASE + FLASH_PAGE_SIZE)
#define OTADATA_COPY_SIZE   (FLASH_PAGE_SIZE)

/* Slot A: 运行槽 */
#define APP_A_BASE          0x08004000U
#define APP_A_SIZE          (16U * 1024U)

/* Slot B: 回滚备份槽 */
#define APP_B_BASE          0x08008000U
#define APP_B_SIZE          (16U * 1024U)

/* Slot C: OTA 下载 staging */
#define APP_C_BASE          0x0800C000U
#define APP_C_SIZE          (16U * 1024U)

/* 编译期一致性检查 */
_Static_assert(BOOT_BASE + BOOT_SIZE == OTADATA_BASE,
               "boot/otadata gap");
_Static_assert(OTADATA_BASE + OTADATA_SIZE == APP_A_BASE,
               "otadata/slotA gap");
_Static_assert(APP_A_BASE + APP_A_SIZE == APP_B_BASE,
               "slotA/slotB gap");
_Static_assert(APP_B_BASE + APP_B_SIZE == APP_C_BASE,
               "slotB/slotC gap");
_Static_assert(APP_C_BASE + APP_C_SIZE
               == FLASH_BASE_ADDR + FLASH_TOTAL_SIZE,
               "flash overflow");
_Static_assert(APP_A_SIZE == APP_B_SIZE && APP_B_SIZE == APP_C_SIZE,
               "slot size must match");
_Static_assert(BOOT_BASE % FLASH_PAGE_SIZE == 0
               && APP_A_BASE % FLASH_PAGE_SIZE == 0
               && APP_B_BASE % FLASH_PAGE_SIZE == 0
               && APP_C_BASE % FLASH_PAGE_SIZE == 0,
               "partition base must be page-aligned");
