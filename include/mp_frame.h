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

mp_result_t mp_frame_parse(const uint8_t *buf, size_t buf_len, mp_frame_t *out);
