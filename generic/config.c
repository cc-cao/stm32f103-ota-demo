#include "config.h"
#include "bsp/flash.h"
#include <stddef.h>

/*
 * otadata 落盘细节
 * ----------------------------------------------------------------
 * 物理: OTADATA_COPY0_BASE / OTADATA_COPY1_BASE 两个 1KB page，
 *       每页只放一份 otadata_t (24B)。
 *
 * 写流程 (config_save):
 *   1. 两份都读出来，选 seq 较小的那份做目标 (轮转写，磨损均匀)
 *   2. 擦目标 page，写新结构 (seq+1, self_crc 重算)
 *   过程中掉电时另一副本仍然完整 → 启动总能恢复出至少一份
 *
 * 读流程 (config_load):
 *   1. 两份都读
 *   2. magic + self_crc 都对算"有效"
 *   3. 都有效 → seq 大者; 一份有效 → 取那份; 都坏 → 返回 false
 */

static uint32_t crc32_update(uint32_t crc, const uint8_t *data, uint32_t len) {
    crc = ~crc;
    while (len--) {
        crc ^= *data++;
        for (int i = 0; i < 8; i++) {
            uint32_t mask = -(int32_t)(crc & 1U);
            crc = (crc >> 1) ^ (0xEDB88320U & mask);
        }
    }
    return ~crc;
}

uint32_t crc32(const uint8_t *data, uint32_t len) {
    return crc32_update(0, data, len);
}

// self_crc 是结构体最后一个字段，CRC 覆盖它前面的所有字节。
static uint32_t self_crc_of(const otadata_t *c) {
    return crc32((const uint8_t *)c, offsetof(otadata_t, self_crc));
}

static bool read_copy(uint32_t base, otadata_t *out) {
    flash_read(base, (uint8_t *)out, sizeof(*out));
    if (out->magic != OTA_MAGIC) {
        return false;
    }
    return out->self_crc == self_crc_of(out);
}

bool config_load(otadata_t *out) {
    otadata_t a, b;
    bool va = read_copy(OTADATA_COPY0_BASE, &a);
    bool vb = read_copy(OTADATA_COPY1_BASE, &b);

    if (va && vb) {
        *out = (a.seq >= b.seq) ? a : b;
        return true;
    }
    if (va) { *out = a; return true; }
    if (vb) { *out = b; return true; }
    return false;
}

bool config_save(const otadata_t *cfg) {
    otadata_t a, b;
    bool va = read_copy(OTADATA_COPY0_BASE, &a);
    bool vb = read_copy(OTADATA_COPY1_BASE, &b);

    // 选 seq 小的那份覆盖；任一份无效就先填无效那份。
    uint32_t target_base;
    uint32_t next_seq;
    if (!va && !vb) {
        target_base = OTADATA_COPY0_BASE;
        next_seq = 1;
    } else if (!va) {
        target_base = OTADATA_COPY0_BASE;
        next_seq = b.seq + 1;
    } else if (!vb) {
        target_base = OTADATA_COPY1_BASE;
        next_seq = a.seq + 1;
    } else {
        target_base = (a.seq <= b.seq) ? OTADATA_COPY0_BASE : OTADATA_COPY1_BASE;
        next_seq = ((a.seq >= b.seq) ? a.seq : b.seq) + 1;
    }

    otadata_t buf = *cfg;
    buf.magic = OTA_MAGIC;
    buf.seq = next_seq;
    buf.self_crc = self_crc_of(&buf);

    flash_page_erase(target_base, OTADATA_COPY_SIZE / FLASH_PAGE_SIZE);
    flash_write(target_base, (uint16_t *)&buf, sizeof(buf) / sizeof(uint16_t));

    // 回读校验，防止 flash 编程出错或被切电。
    otadata_t verify;
    return read_copy(target_base, &verify) && verify.seq == next_seq;
}
