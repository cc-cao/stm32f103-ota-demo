#include "ota.h"
#include "bsp/usart.h"
#include "bsp/flash.h"
#include "config.h"
#include "partition.h"
#include "stm32f1xx_hal.h"
#include <string.h>

/*
 * 接收链路 (HAL 空闲中断):
 *   HAL_UARTEx_ReceiveToIdle_IT 启动一次接收 →
 *   收到 IDLE / buffer 满 → HAL_UARTEx_RxEventCallback(size) →
 *   置 rx_ready=true，main loop 调 ota_poll() 解析 →
 *   解析完再启一次 ReceiveToIdle_IT 接收下一帧
 *
 * 协议帧格式:
 *   [SOF=0xAA][LEN:2 LE][CMD:1][SEQ:1][PAYLOAD:N][CRC16:2 LE]
 *   LEN = CMD+SEQ+PAYLOAD+CRC 的字节数，CRC 覆盖 LEN..PAYLOAD
 */

#define RX_BUF_SIZE 320U   /* > 一帧最大 (5 + 256 + 2 = 263) */

#define SOF        0xAAU
#define CMD_BEGIN  0x02U
#define CMD_DATA   0x03U
#define CMD_END    0x04U
#define CMD_ACK    0x80U
#define CMD_NAK    0xEEU

#define NAK_BAD_CRC      0x01U
#define NAK_BAD_STATE    0x02U
#define NAK_BAD_OFFSET   0x03U
#define NAK_FLASH_FAIL   0x04U
#define NAK_TOO_BIG      0x05U

typedef enum {
    S_IDLE,
    S_RECEIVING,
} state_t;

static uint8_t  rx_buf[RX_BUF_SIZE];
static volatile uint16_t rx_len;
static volatile bool     rx_ready;

static state_t   state;
static uint32_t  expected_size;
static uint32_t  expected_crc;
static uint32_t  received_size;
static bool      verify_stage_at_boot;

static uint16_t crc16_ccitt(const uint8_t *data, uint32_t len) {
    uint16_t crc = 0xFFFFU;
    while (len--) {
        crc ^= (uint16_t)(*data++) << 8;
        for (int i = 0; i < 8; i++) {
            crc = (crc & 0x8000U) ? (uint16_t)((crc << 1) ^ 0x1021U) : (uint16_t)(crc << 1);
        }
    }
    return crc;
}

static void send_raw(uint8_t cmd, uint8_t seq, const uint8_t *payload, uint16_t plen) {
    uint8_t hdr[5];
    hdr[0] = SOF;
    uint16_t len_field = (uint16_t)(2U + plen + 2U);
    hdr[1] = (uint8_t)(len_field & 0xFFU);
    hdr[2] = (uint8_t)(len_field >> 8);
    hdr[3] = cmd;
    hdr[4] = seq;

    /* 拼一份连续 buffer 算 CRC，避免分段拼接的复杂度 */
    uint8_t tmp[4 + OTA_MAX_PAYLOAD];
    memcpy(tmp, &hdr[1], 4U);
    if (plen) memcpy(tmp + 4U, payload, plen);
    uint16_t crc = crc16_ccitt(tmp, 4U + plen);

    uint8_t crc_le[2] = {(uint8_t)(crc & 0xFFU), (uint8_t)(crc >> 8)};

    HAL_UART_Transmit(&huart1, hdr, sizeof(hdr), HAL_MAX_DELAY);
    if (plen) HAL_UART_Transmit(&huart1, (uint8_t *)payload, plen, HAL_MAX_DELAY);
    HAL_UART_Transmit(&huart1, crc_le, 2, HAL_MAX_DELAY);
}

static void send_ack(uint8_t orig_cmd, uint8_t seq, uint8_t status) {
    send_raw((uint8_t)(CMD_ACK | orig_cmd), seq, &status, 1U);
}

static void send_nak(uint8_t seq, uint8_t reason) {
    send_raw(CMD_NAK, seq, &reason, 1U);
}

void ota_init(void) {
    state = S_IDLE;
    rx_len = 0;
    rx_ready = false;

    otadata_t cfg;
    verify_stage_at_boot = config_load(&cfg) && cfg.stage == OTA_VERIFY;

    HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_buf, sizeof(rx_buf));
}

bool ota_in_verify_stage(void) {
    return verify_stage_at_boot;
}

void ota_mark_valid(void) {
    otadata_t cfg = {0};
    config_load(&cfg);
    cfg.stage = OTA_IDLE;
    config_save(&cfg);
    verify_stage_at_boot = false;
}

/* HAL 在 IDLE/buffer 满时调这个，Size 是这一帧实际收到的字节数 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {
    if (huart->Instance != USART1) return;

    if (!rx_ready && Size > 0) {
        rx_len = Size;
        rx_ready = true;
        /* 不立刻重新 arm: 等 ota_poll 处理完 rx_buf 再启，避免缓冲被新数据覆盖 */
    } else {
        /* 上一帧还没消化 / 空触发: 直接重新 arm，丢这一帧 (上位机超时重传) */
        HAL_UARTEx_ReceiveToIdle_IT(huart, rx_buf, sizeof(rx_buf));
    }
}

/* HAL 错误回调: ORE (overrun) 必须复位 RxState 否则后续接收都失败 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance != USART1) return;

    if (huart->ErrorCode & HAL_UART_ERROR_ORE) {
        __HAL_UART_CLEAR_OREFLAG(huart);
        huart->RxState = HAL_UART_STATE_READY;
        __HAL_UNLOCK(huart);
        HAL_UARTEx_ReceiveToIdle_IT(huart, rx_buf, sizeof(rx_buf));
    }
}

static void handle_begin(uint8_t seq, const uint8_t *p, uint16_t plen) {
    if (plen < 12U) { send_nak(seq, NAK_BAD_CRC); return; }
    uint32_t total = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    uint32_t crc   = (uint32_t)p[4] | ((uint32_t)p[5] << 8) | ((uint32_t)p[6] << 16) | ((uint32_t)p[7] << 24);

    if (total > APP_C_SIZE) { send_nak(seq, NAK_TOO_BIG); return; }

    /* 擦 C 槽: 16 page × ~30ms ≈ 480ms。
     * 如果你的 IWDG 周期太短可能会被咬，需要的话拆成"擦一页喂一次狗"。 */
    flash_page_erase(APP_C_BASE, APP_C_SIZE / FLASH_PAGE_SIZE);

    expected_size = total;
    expected_crc  = crc;
    received_size = 0;
    state = S_RECEIVING;

    otadata_t cfg = {0};
    config_load(&cfg);
    cfg.stage = OTA_DOWNLOADING;
    config_save(&cfg);

    send_ack(CMD_BEGIN, seq, 0);
}

static void handle_data(uint8_t seq, const uint8_t *p, uint16_t plen) {
    if (state != S_RECEIVING) { send_nak(seq, NAK_BAD_STATE); return; }
    if (plen < 4U) { send_nak(seq, NAK_BAD_CRC); return; }

    uint32_t offset = (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
    const uint8_t *data = p + 4;
    uint16_t dlen = (uint16_t)(plen - 4U);

    if (offset != received_size) { send_nak(seq, NAK_BAD_OFFSET); return; }
    if ((uint64_t)offset + dlen > expected_size) { send_nak(seq, NAK_TOO_BIG); return; }

    /* flash 是半字编程，dlen 必须 2 字节对齐；最后一帧不足两字节就用 0xFF 补 */
    uint16_t aligned = (uint16_t)((dlen + 1U) & ~1U);
    static uint8_t pad_buf[OTA_MAX_PAYLOAD + 2];
    memcpy(pad_buf, data, dlen);
    if (aligned > dlen) pad_buf[dlen] = 0xFF;

    flash_write(APP_C_BASE + offset, (uint16_t *)pad_buf, aligned / 2U);

    received_size += dlen;

    /* ACK 带 offset 让上位机能确认到哪了，方便断点续传 */
    uint8_t resp[5];
    resp[0] = (uint8_t)(offset & 0xFFU);
    resp[1] = (uint8_t)((offset >> 8) & 0xFFU);
    resp[2] = (uint8_t)((offset >> 16) & 0xFFU);
    resp[3] = (uint8_t)((offset >> 24) & 0xFFU);
    resp[4] = 0;
    send_raw((uint8_t)(CMD_ACK | CMD_DATA), seq, resp, sizeof(resp));
}

static void handle_end(uint8_t seq) {
    if (state != S_RECEIVING) { send_nak(seq, NAK_BAD_STATE); return; }
    if (received_size != expected_size) { send_nak(seq, NAK_BAD_OFFSET); return; }

    /* 整片 CRC 校验，不通过就当本次升级作废 */
    uint32_t actual_crc = crc32((const uint8_t *)APP_C_BASE, expected_size);
    if (actual_crc != expected_crc) {
        state = S_IDLE;
        otadata_t cfg = {0};
        config_load(&cfg);
        cfg.stage = OTA_IDLE;
        config_save(&cfg);
        send_nak(seq, NAK_BAD_CRC);
        return;
    }

    /* 标记 PENDING，记录目标镜像信息，重启交给 bootloader 应用升级 */
    otadata_t cfg = {0};
    config_load(&cfg);
    cfg.stage = OTA_PENDING;
    cfg.image_size = expected_size;
    cfg.image_crc = expected_crc;
    config_save(&cfg);

    send_ack(CMD_END, seq, 0);

    HAL_Delay(50);    /* 让 ACK 物理发完 */
    NVIC_SystemReset();
}

static void parse_one_frame(const uint8_t *buf, uint16_t len) {
    /* 容忍前面有零碎噪声: 扫描第一个 SOF */
    uint16_t i = 0;
    while (i < len && buf[i] != SOF) i++;
    if (i + 5U >= len) return;

    const uint8_t *frame = buf + i;
    uint16_t remaining = (uint16_t)(len - i);
    uint16_t len_field = (uint16_t)frame[1] | ((uint16_t)frame[2] << 8);
    if (len_field < 4U || len_field > (2U + 4U + OTA_MAX_PAYLOAD)) return;
    if ((uint16_t)(3U + len_field) > remaining) return;

    /* CRC 覆盖 LEN(2) + CMD(1) + SEQ(1) + PAYLOAD，不含末尾的 CRC 自己 */
    uint16_t check_len = (uint16_t)(2U + len_field - 2U);
    uint16_t calc = crc16_ccitt(&frame[1], check_len);
    uint16_t got  = (uint16_t)frame[3 + len_field - 2] | ((uint16_t)frame[3 + len_field - 1] << 8);
    if (calc != got) return;   /* 静默丢，上位机超时重传 */

    uint8_t cmd = frame[3];
    uint8_t seq = frame[4];
    const uint8_t *payload = &frame[5];
    uint16_t plen = (uint16_t)(len_field - 4U);

    switch (cmd) {
    case CMD_BEGIN: handle_begin(seq, payload, plen); break;
    case CMD_DATA:  handle_data(seq, payload, plen); break;
    case CMD_END:   handle_end(seq); break;
    default:        send_nak(seq, NAK_BAD_STATE); break;
    }
}

void ota_poll(void) {
    if (!rx_ready) return;

    parse_one_frame(rx_buf, rx_len);

    rx_ready = false;
    rx_len = 0;
    /* 解析完再启接收，避免 buffer 被新一帧覆盖 */
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, rx_buf, sizeof(rx_buf));
}
