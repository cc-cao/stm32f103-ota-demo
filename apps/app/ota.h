#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "generic.h"
#include <stdbool.h>

/*
 * App 端 OTA 模块
 *
 * 工作模式:
 *   - usart1 + DMA + IDLE 中断接收变长帧 (零 CPU 开销)
 *   - 帧解析在 main loop 调用 ota_poll() 里完成 (不在中断里写 flash)
 *   - 写 flash 期间 DMA 仍然能收，但 stop-and-wait 协议保证此刻上位机不发
 *
 * 帧格式:
 *   [SOF=0xAA][LEN:2 LE][CMD:1][SEQ:1][PAYLOAD:N][CRC16:2 LE]
 *   LEN = CMD+SEQ+PAYLOAD+CRC 的总字节数 (不含 SOF 和 LEN 自己)
 *
 * 命令字:
 *   0x02 BEGIN       payload: total_size:4 + image_crc32:4 + version:4
 *   0x03 DATA        payload: offset:4 + bytes...
 *   0x04 END         payload: 无
 *   0x80|cmd ACK     payload: 视命令而定，通常 status:1
 *   0xEE NAK         payload: error_code:1
 *
 * 启动后调用顺序:
 *   ota_init();                  // 初始化 USART1 + DMA + IDLE
 *   if (ota_in_verify_stage())   // 检查是不是新固件试运行
 *       ...业务跑稳后... ota_mark_valid();
 *   while (1) {
 *       业务...
 *       ota_poll();              // 处理收到的帧
 *       iwdg_feed();
 *   }
 */

#define OTA_MAX_PAYLOAD 256U

void ota_init(void);

/* 主循环里反复调，处理 RX buffer 里的帧。不阻塞。*/
void ota_poll(void);

/* 当前启动是不是 bootloader 跳过来的"试运行"。是的话业务必须在合理时间内调 ota_mark_valid()。*/
bool ota_in_verify_stage(void);

/* 试运行通过，告诉 bootloader 新固件可用 (stage=IDLE)。*/
void ota_mark_valid(void);

#ifdef __cplusplus
}
#endif
