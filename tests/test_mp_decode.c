#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "big_endian.h"
#include "mp_decode.h"
#include "mp_frame.h"
#include "protocol.h"

static void make_measure_frame(const uint8_t *payload, uint16_t payload_len, mp_frame_t *out) {
    if (payload == NULL || out == NULL) {
        return;
    }
    out->version = MP_VERSION;
    out->msg_type = MP_MSG_TYPE_MEASURE;
    out->seq = 0;
    out->payload_len = payload_len;
    memcpy(out->payload, payload, payload_len);

    // crc 已於 parser 階段檢驗過, decode 階段略過
    out->crc = 0;
}

static size_t build_measure_payload(
    uint32_t timestamp,
    uint8_t channel_count,
    const mp_reading_t *readings,
    uint8_t *out
) {
    big_endian_write_uint32(out, timestamp);
    out[4] = channel_count;

    mp_reading_t reading = {0};

    for (size_t i = 0; i < channel_count; i++) {
        size_t base = 5 + 6 * i;
        reading = readings[i];

        out[base] = reading.channel_id;
        out[base + 1] = reading.unit;
        big_endian_write_uint32(&out[base + 2], (uint32_t)reading.value_milli);
    }
    return (size_t)(5 + 6 * channel_count);
}

static void test_one_channel(void) {
    uint8_t buf[256] = {0};
    mp_reading_t reading[] = {
        {
            .channel_id = 5,
            .unit = MP_UNIT_VOLT,
            .value_milli = 12500,
        },
    };
    size_t payload_len = build_measure_payload(1000, 1, reading, buf);

    mp_frame_t f = {0};
    make_measure_frame(buf, (uint16_t)payload_len, &f);

    mp_measurement_t m = {0};
    mp_result_t result = mp_decode_measurement(&f, &m);

    assert(result == MP_RESULT_OK);
    assert(m.timestamp_ms == 1000);
    assert(m.channel_count == 1);
    assert(m.readings[0].channel_id == 5);
    assert(m.readings[0].unit == MP_UNIT_VOLT);
    assert(m.readings[0].value_milli == 12500);
}

static void test_two_channel(void) {

    uint8_t buf[256] = {0};
    mp_reading_t reading[] = {
        {
            .channel_id = 1,
            .unit = MP_UNIT_VOLT,
            .value_milli = 5000,
        },
        {
            .channel_id = 2,
            .unit = MP_UNIT_CELSIUS,
            .value_milli = -3200,
        },
    };
    size_t payload_len = build_measure_payload(2000, 2, reading, buf);

    mp_frame_t f = {0};
    make_measure_frame(buf, (uint16_t)payload_len, &f);

    mp_measurement_t m = {0};
    mp_result_t result = mp_decode_measurement(&f, &m);

    assert(result == MP_RESULT_OK);
    assert(m.timestamp_ms == 2000);
    assert(m.channel_count == 2);

    assert(m.readings[0].channel_id == 1);
    assert(m.readings[0].unit == MP_UNIT_VOLT);
    assert(m.readings[0].value_milli == 5000);

    assert(m.readings[1].channel_id == 2);
    assert(m.readings[1].unit == MP_UNIT_CELSIUS);
    assert(m.readings[1].value_milli == -3200);
}

static void test_zero_channel(void) {
    uint8_t buf[256] = {0};
    size_t payload_len = build_measure_payload(3000, 0, NULL, buf);

    mp_frame_t f = {0};
    make_measure_frame(buf, (uint16_t)payload_len, &f);

    mp_measurement_t m = {0};
    mp_result_t result = mp_decode_measurement(&f, &m);

    assert(result == MP_RESULT_OK);
    assert(m.timestamp_ms == 3000);
    assert(m.channel_count == 0);
}
static void test_msg_type_not_measure(void) {
    uint8_t buf[256] = {0};
    size_t payload_len = build_measure_payload(4000, 0, NULL, buf);

    mp_frame_t f = {0};
    make_measure_frame(buf, (uint16_t)payload_len, &f);
    f.msg_type = MP_MSG_TYPE_ACK;

    mp_measurement_t m = {0};
    mp_result_t result = mp_decode_measurement(&f, &m);

    assert(result == MP_RESULT_ERROR_PAYLOAD);
    assert(m.timestamp_ms == 0);
    assert(m.channel_count == 0);
}

static void test_payload_len_and_channel_not_fit(void) {
    uint8_t buf[256] = {0};
    mp_reading_t readings[1] = {
        {
            .channel_id = 1,
            .unit = MP_UNIT_VOLT,
            .value_milli = 5000,
        },
    };
    size_t payload_len = build_measure_payload(5000, 1, readings, buf);
    mp_frame_t f = {0};
    make_measure_frame(buf, (uint16_t)payload_len, &f);
    f.payload_len = 20;

    mp_measurement_t m = {0};
    mp_result_t result = mp_decode_measurement(&f, &m);

    assert(result == MP_RESULT_ERROR_PAYLOAD);
    assert(m.timestamp_ms == 0);
    assert(m.channel_count == 0);
}

static void test_big_channel_count(void) {
    mp_frame_t f = {0};
    f.version = MP_VERSION;
    f.msg_type = MP_MSG_TYPE_MEASURE;
    f.payload_len = (uint16_t)(5 + (MP_MAX_CHANNELS + 1) * 6);
    f.payload[4] = MP_MAX_CHANNELS + 1;

    mp_measurement_t m = {0};
    mp_result_t result = mp_decode_measurement(&f, &m);

    assert(result == MP_RESULT_ERROR_PAYLOAD);
    assert(m.timestamp_ms == 0);
    assert(m.channel_count == 0);
}

static void test_small_payload_len(void) {
    uint8_t buf[256] = {0};
    mp_frame_t f = {0};
    make_measure_frame(buf, 4, &f);

    mp_measurement_t m = {0};
    mp_result_t result = mp_decode_measurement(&f, &m);
    assert(result == MP_RESULT_ERROR_PAYLOAD);
    assert(m.timestamp_ms == 0);
    assert(m.channel_count == 0);
}

int main(void) {
    test_one_channel();
    test_two_channel();
    test_zero_channel();
    test_msg_type_not_measure();
    test_payload_len_and_channel_not_fit();
    test_big_channel_count();
    test_small_payload_len();
    return 0;
}
