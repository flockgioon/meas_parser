#include <assert.h>
#include <string.h>

#include "big_endian.h"
#include "crc.h"
#include "mp_frame.h"
#include "mp_parser.h"
#include "protocol.h"

static size_t build_frame(
    mp_msg_type_t msg_type,
    uint16_t seq,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *out_buf
) {
    out_buf[0] = MP_SYNC0;
    out_buf[1] = MP_SYNC1;
    out_buf[2] = MP_VERSION;
    out_buf[3] = (uint8_t)msg_type;
    big_endian_write_uint16(&out_buf[4], seq);
    big_endian_write_uint16(&out_buf[6], payload_len);
    if (payload_len > 0) {
        memcpy(&out_buf[8], payload, payload_len);
    }
    uint16_t crc = crc_crc16_ccitt_false(&out_buf[2], 6 + payload_len);
    big_endian_write_uint16(&out_buf[MP_HEADER_LEN + payload_len], crc);

    return (size_t)(MP_HEADER_LEN + payload_len + MP_CRC_LEN);
}

static void test_single_frame(void) {
    mp_parser_t p = {0};
    mp_parser_init(&p);

    mp_frame_t f = {0};

    uint8_t payload[] = {0xAB, 0xCD, 0xEF};
    uint8_t raw_buf[64] = {0};
    size_t n = build_frame(MP_MSG_TYPE_MEASURE, 48763, payload, 3, raw_buf);
    assert(mp_parser_feed(&p, raw_buf, n) == n);
    assert(mp_parser_consume(&p, &f) == MP_RESULT_OK);
    assert(f.msg_type == MP_MSG_TYPE_MEASURE);
    assert(f.seq == 48763);
    assert(f.payload_len == 3);
    assert(f.payload[0] == 0xAB);
    assert(f.payload[1] == 0xCD);
    assert(f.payload[2] == 0xEF);

    // parser should be empty and can not consume
    assert(p.len == 0);
    assert(mp_parser_consume(&p, &f) == MP_RESULT_NEED_MORE_DATA);
}

int main(void) {
    test_single_frame();
    return 0;
}
