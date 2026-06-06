#pragma once

#include <stdint.h>

#include "protocol.h"

typedef struct {
    uint8_t version;
    uint8_t msg_type;
    uint16_t seq;
    uint16_t payload_len;
    uint8_t payload[MP_MAX_PAYLOAD];
    uint16_t crc;
} mp_frame_t;

/*
 * 從 "包含完整封包" 的 buffer 解析出 frame
 * buf: 原始 bytes
 * buf_len: buf 的總長度
 * out: 若解析成功將把結果寫入 out
 * return: 回傳 MP_RESULT_* 作為結果標示、錯誤碼
 */
mp_result_t mp_frame_parse(const uint8_t *buf, size_t buf_len, mp_frame_t *out);
