#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "mp_frame.h"
#include "mp_parser.h"
#include "protocol.h"
#include "build_helper.h"

#define MP_RESULT_INIT 999

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

static void test_byte_feed(void) {
    mp_parser_t p = {0};
    mp_parser_init(&p);

    mp_frame_t f = {0};

    uint8_t payload[] = {0xAB, 0xCD, 0xEF};
    uint8_t raw_buf[64] = {0};
    size_t n = build_frame(MP_MSG_TYPE_MEASURE, 13579, payload, 3, raw_buf);
    for (size_t i = 0; i < n - 1; i++) {
        assert(mp_parser_feed(&p, &raw_buf[i], 1) == 1);
        assert(mp_parser_consume(&p, &f) == MP_RESULT_NEED_MORE_DATA);
    }
    assert(mp_parser_feed(&p, &raw_buf[n - 1], 1) == 1);
    assert(mp_parser_consume(&p, &f) == MP_RESULT_OK);
    assert(f.msg_type == MP_MSG_TYPE_MEASURE);
    assert(f.seq == 13579);
    assert(f.payload_len == 3);
    assert(f.payload[0] == 0xAB);
    assert(f.payload[1] == 0xCD);
    assert(f.payload[2] == 0xEF);

    // parser should be empty and can not consume
    assert(p.len == 0);
    assert(mp_parser_consume(&p, &f) == MP_RESULT_NEED_MORE_DATA);
}

static void test_two_frames(void) {

    mp_parser_t p = {0};
    mp_parser_init(&p);

    mp_frame_t f = {0};
    uint8_t payload1[] = {0x01, 0x02, 0x03};
    uint8_t payload2[] = {0x04, 0x05, 0x06};
    uint8_t raw_buf[64] = {0};
    size_t n1 = build_frame(MP_MSG_TYPE_MEASURE, 1, payload1, 3, raw_buf);
    size_t n2 = build_frame(MP_MSG_TYPE_MEASURE, 2, payload2, 3, &raw_buf[n1]);
    mp_result_t result = MP_RESULT_INIT;

    assert(mp_parser_feed(&p, raw_buf, n1 + n2) == n1 + n2);
    // frame 1
    result = mp_parser_consume(&p, &f);
    assert(result == MP_RESULT_OK);
    assert(f.msg_type == MP_MSG_TYPE_MEASURE);
    assert(f.seq == 1);
    assert(f.payload_len == 3);
    assert(f.payload[0] == 0x01);
    assert(f.payload[1] == 0x02);
    assert(f.payload[2] == 0x03);

    assert(p.len == n2);

    // frame 2
    result = mp_parser_consume(&p, &f);
    assert(result == MP_RESULT_OK);
    assert(f.msg_type == MP_MSG_TYPE_MEASURE);
    assert(f.seq == 2);
    assert(f.payload_len == 3);
    assert(f.payload[0] == 0x04);
    assert(f.payload[1] == 0x05);
    assert(f.payload[2] == 0x06);

    assert(p.len == 0);

    // parser should be empty and can not consume
    assert(p.len == 0);
    assert(mp_parser_consume(&p, &f) == MP_RESULT_NEED_MORE_DATA);
}

static void test_with_junk(void) {

    mp_parser_t p = {0};
    mp_parser_init(&p);

    mp_frame_t f = {0};

    uint8_t junk[] = {0xAA, 0xBB, 0xCC};
    mp_parser_feed(&p, junk, 3);
    uint8_t raw_buf[64] = {0};
    size_t n = build_frame(MP_MSG_TYPE_ACK, 55, NULL, 0, raw_buf);
    assert(mp_parser_feed(&p, raw_buf, n) == n);
    for (int i = 0; i < 3; i++) {
        assert(mp_parser_consume(&p, &f) == MP_RESULT_RESYNC);
    }
    assert(mp_parser_consume(&p, &f) == MP_RESULT_OK);
    assert(f.msg_type == MP_MSG_TYPE_ACK);
    assert(f.seq == 55);
    assert(f.payload_len == 0);

    // parser should be empty and can not consume
    assert(p.len == 0);
    assert(mp_parser_consume(&p, &f) == MP_RESULT_NEED_MORE_DATA);
}

static void test_bad_crc_and_good_crc(void) {

    mp_parser_t p = {0};
    mp_parser_init(&p);

    mp_frame_t f = {0};

    uint8_t payload1[] = {0x77, 0x88, 0x99};
    uint8_t payload2[] = {0xAA, 0xBB, 0xCC};
    uint8_t raw_buf[64] = {0};
    size_t n1 = build_frame(MP_MSG_TYPE_MEASURE, 55, payload1, 3, raw_buf);
    // edit first frame buffer
    raw_buf[8] = 0x55;
    size_t n2 = build_frame(MP_MSG_TYPE_MEASURE, 66, payload2, 3, &raw_buf[n1]);

    assert(mp_parser_feed(&p, raw_buf, n1 + n2) == n1 + n2);
    assert(mp_parser_consume(&p, &f) == MP_RESULT_ERROR_CRC);
    assert(mp_parser_consume(&p, &f) == MP_RESULT_OK);
    assert(f.msg_type == MP_MSG_TYPE_MEASURE);
    assert(f.seq == 66);
    assert(f.payload_len == 3);
    assert(f.payload[0] == 0xAA);
    assert(f.payload[1] == 0xBB);
    assert(f.payload[2] == 0xCC);

    // parser should be empty and can not consume
    assert(p.len == 0);
    assert(mp_parser_consume(&p, &f) == MP_RESULT_NEED_MORE_DATA);
}

int main(void) {
    test_single_frame();
    test_byte_feed();
    test_two_frames();
    test_with_junk();
    test_bad_crc_and_good_crc();
    return 0;
}
