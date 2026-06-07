#include <string.h>

#include "big_endian.h"
#include "mp_frame.h"
#include "mp_parser.h"
#include "protocol.h"

void mp_parser_init(mp_parser_t *p) {
    if (p != NULL) {
        p->len = 0;
    }
}

size_t mp_parser_feed(mp_parser_t *p, const uint8_t *bytes, size_t n) {
    if (p == NULL || bytes == NULL || n == 0) {
        return 0;
    }

    size_t rest_space = MP_PARSER_BUF_SIZE - p->len;
    size_t can_take = rest_space >= n ? n : rest_space;
    memcpy(&p->data[p->len], bytes, can_take);
    p->len += can_take;
    return can_take;
}

mp_result_t mp_parser_consume(mp_parser_t *p, mp_frame_t *out) {
    if (p == NULL || out == NULL) {
        return MP_RESULT_ERROR_INVALID_ARGS;
    }

    if (p->len < 2) {
        return MP_RESULT_NEED_MORE_DATA;
    }
    if (p->data[0] != MP_SYNC0 || p->data[1] != MP_SYNC1) {
        memmove(p->data, &p->data[1], --p->len);
        return MP_RESULT_RESYNC;
    }

    if (p->len < MP_HEADER_LEN) {
        return MP_RESULT_NEED_MORE_DATA;
    }
    uint16_t payload_len = big_endian_uint16(&p->data[6]);
    if (payload_len > MP_MAX_PAYLOAD) {
        memmove(p->data, &p->data[1], --p->len);
        return MP_RESULT_ERROR_LENGTH;
    }

    size_t total_len = MP_HEADER_LEN + payload_len + MP_CRC_LEN;
    if (p->len < total_len) {
        return MP_RESULT_NEED_MORE_DATA;
    }

    mp_result_t result = mp_frame_parse(p->data, total_len, out);
    p->len -= total_len;
    memmove(p->data, &p->data[total_len], p->len);
    return result;
}
