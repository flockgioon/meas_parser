#include <string.h>

#include "build_helper.h"
#include "big_endian.h"
#include "crc.h"

size_t build_frame(
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

size_t build_measure_payload(
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

void make_measure_frame(const uint8_t *payload, uint16_t payload_len, mp_frame_t *out) {
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

