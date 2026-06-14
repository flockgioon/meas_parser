#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "big_endian.h"
#include "crc.h"
#include "mp_decode.h"
#include "protocol.h"
#include "portable.h"

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

static void write_junk(FILE *f) {
    if (f == NULL) {
        return;
    }

    uint8_t frame[256] = {0};
    uint8_t junk[] = {0xAA, 0xBB, 0xCC};
    fwrite(frame, 1, sizeof(junk), f);
}

static void write_valid_measure_frame_with_2_channel(FILE *f) {
    mp_reading_t readings[] = {
        (mp_reading_t){
            .channel_id = 1,
            .unit = MP_UNIT_VOLT,
            .value_milli = 3000,
        },
        (mp_reading_t){
            .channel_id = 2,
            .unit = MP_UNIT_CELSIUS,
            .value_milli = -1500,
        },
    };
    uint8_t payload[128] = {0};
    uint8_t frame[256] = {0};
    size_t payload_len = build_measure_payload(1000, 2, readings, payload);
    size_t frame_len = build_frame(MP_MSG_TYPE_MEASURE, 111, payload, (uint16_t)payload_len, frame);
    fwrite(frame, 1, frame_len, f);
}

static void write_valid_measure_frame_with_1_channel(FILE *f) {
    mp_reading_t readings[] = {
        (mp_reading_t){
            .channel_id = 3,
            .unit = MP_UNIT_AMP,
            .value_milli = 500,
        },
    };
    uint8_t payload[128] = {0};
    uint8_t frame[256] = {0};
    size_t payload_len = build_measure_payload(2000, 1, readings, payload);
    size_t frame_len = build_frame(MP_MSG_TYPE_MEASURE, 222, payload, (uint16_t)payload_len, frame);
    fwrite(frame, 1, frame_len, f);
}

static void write_bad_crc(FILE *f) {
    mp_reading_t readings[] = {
        (mp_reading_t){
            .channel_id = 4,
            .unit = MP_UNIT_VOLT,
            .value_milli = 2500,
        },
    };
    uint8_t payload[128] = {0};
    uint8_t frame[256] = {0};
    size_t payload_len = build_measure_payload(3000, 1, readings, payload);
    size_t frame_len = build_frame(MP_MSG_TYPE_MEASURE, 333, payload, (uint16_t)payload_len, frame);
    frame[10] ^= 0x01;
    fwrite(frame, 1, frame_len, f);
}

static void write_ack_frame(FILE *f) {
    uint8_t frame[256] = {0};
    size_t frame_len = build_frame(MP_MSG_TYPE_ACK, 444, NULL, 0, frame);
    fwrite(frame, 1, frame_len, f);
}

static void write_valid_measure_frame_with_3_channel(FILE *f) {
    mp_reading_t readings[] = {
        (mp_reading_t){
            .channel_id = 11,
            .unit = MP_UNIT_VOLT,
            .value_milli = 24800,
        },
        (mp_reading_t){
            .channel_id = 12,
            .unit = MP_UNIT_CELSIUS,
            .value_milli = -86400,
        },
        (mp_reading_t){
            .channel_id = 13,
            .unit = MP_UNIT_AMP,
            .value_milli = -50,
        },
    };
    uint8_t payload[128] = {0};
    uint8_t frame[256] = {0};
    size_t payload_len = build_measure_payload(36000000, 3, readings, payload);
    size_t frame_len = build_frame(MP_MSG_TYPE_MEASURE, 555, payload, (uint16_t)payload_len, frame);
    fwrite(frame, 1, frame_len, f);
}

int main(void) {
    const char *FILENAME = "test_data.bin";
    FILE *f = NULL;
    int err = portable_fopen(&f, FILENAME, "wb");
    if (err != 0) {
        fprintf(stderr, "failed to fopen_s: %d\n", err);
        return 1;
    }

    write_junk(f);
    write_valid_measure_frame_with_2_channel(f);
    write_valid_measure_frame_with_1_channel(f);
    write_bad_crc(f);
    write_ack_frame(f);
    write_valid_measure_frame_with_3_channel(f);

    fclose(f);
    printf("wrote %s\n", FILENAME);
    return 0;
}
