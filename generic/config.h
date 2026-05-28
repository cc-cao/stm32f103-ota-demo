#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "generic.h"
#include "partition.h"
#include <stdbool.h>

#define OTA_MAGIC 0xC0DECAFEU

typedef enum {
    OTA_IDLE = 0,
    OTA_DOWNLOADING = 1,
    OTA_PENDING = 2,
    OTA_VERIFY = 3,
} ota_stage_t;

/*
 * otadata 双副本里持久化的结构。共 24 字节，调用方只填 stage / image_size /
 * image_crc，magic/seq/self_crc 由 config_save 维护。
 */
typedef struct {
    uint32_t magic;
    uint32_t seq;
    uint32_t stage;
    uint32_t image_size;
    uint32_t image_crc;
    uint32_t self_crc;
} otadata_t;

/* 启动时调用。返回 false 表示双副本都无效 (例如出厂首次)，调用方用默认值。*/
bool config_load(otadata_t *out);

/* 写入到 seq 较小的那个副本，自动 +1 seq、填 magic、算 self_crc。*/
bool config_save(const otadata_t *cfg);

/* IEEE 802.3 CRC32，给上层算镜像 CRC 用，与 self_crc 共用同一份实现。*/
uint32_t crc32(const uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif
