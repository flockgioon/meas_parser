#pragma once

#include "mp_frame.h"
#include "protocol.h"

#define MP_PARSER_BUF_SIZE (MP_HEADER_LEN + MP_MAX_PAYLOAD + MP_CRC_LEN + 256)

typedef struct {
    uint8_t data[MP_PARSER_BUF_SIZE];
    size_t len;
} mp_parser_t;

void mp_parser_init(mp_parser_t *p);

// append new bytes to parser's buffer
// 新增 new bytes 到 parser 的 buffer
// return: 實際加入到 parser 的 buffer 的量
// return: actual size append to parser's buffer
size_t mp_parser_feed(mp_parser_t *p, const uint8_t *bytes, size_t n);

// 嘗試從 parser 中取出一個 valid frame 並消耗
mp_result_t mp_parser_consume(mp_parser_t *p, mp_frame_t *out);
