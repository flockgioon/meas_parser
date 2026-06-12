#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "mp_decode.h"
#include "mp_frame.h"
#include "mp_log.h"
#include "mp_parser.h"
#include "protocol.h"

#define MP_RESULT_INIT 999

static const char *mp_unit_to_string(mp_unit_t unit) {
    switch (unit) {
    case MP_UNIT_VOLT:
        return "V";
    case MP_UNIT_AMP:
        return "A";
    case MP_UNIT_CELSIUS:
        return "°C";
    default:
        return "(unknown unit)";
    }
}

static void handle_by_result(mp_result_t result, mp_frame_t *frame, size_t *frames_ok, size_t *frames_err, size_t *resync_count) {
    if (result == MP_RESULT_RESYNC) {
        *resync_count += 1;
        return;
    }

    if (result != MP_RESULT_OK) {
        mp_log(MP_LOG_LEVEL_WARN, "seq %u: error %d", frame->seq, result);
        *frames_err += 1;
        return;
    }

    *frames_ok += 1;
    mp_log(MP_LOG_LEVEL_INFO, "seq %u: type=%u, payload_len=%u", frame->seq, frame->msg_type, frame->payload_len);
    if (frame->msg_type != MP_MSG_TYPE_MEASURE) {
        return;
    }

    mp_measurement_t measurement = {0};
    if (mp_decode_measurement(frame, &measurement) != MP_RESULT_OK) {
        return;
    }

    mp_log(MP_LOG_LEVEL_INFO, " timestamp=%u, channel_count=%u", measurement.timestamp_ms, measurement.channel_count);
    for (size_t i = 0; i < measurement.channel_count; i++) {
        mp_reading_t *reading = &measurement.readings[i];
        mp_log(MP_LOG_LEVEL_INFO, " channel=%u, %.3f%s", reading->channel_id, reading->value_milli / 1000.0, mp_unit_to_string(reading->unit));
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "usage %s <file.bin>\n", argv[0]);
        return 1;
    }

    // set log level
    mp_log_set_level(MP_LOG_LEVEL_DEBUG);

    // read file
    FILE *file = NULL;
    errno_t err = fopen_s(&file, argv[1], "rb");
    if (err != 0) {
        fprintf(stderr, "failed to fopen_s: %d\n", err);
        return 1;
    }

    // mp_parser
    mp_parser_t parser = {0};
    mp_parser_init(&parser);

    // for stats
    size_t frames_ok = 0;
    size_t frames_err = 0;
    size_t resync_count = 0;

    // read loop
    uint8_t buf[256] = {0};
    size_t n = 0;
    while ((n = fread(buf, 1, sizeof(buf), file)) > 0) {
        mp_parser_feed(&parser, buf, n);

        mp_frame_t frame = {0};
        mp_result_t result = MP_RESULT_INIT;

        while (1) {
            result = mp_parser_consume(&parser, &frame);
            if (result == MP_RESULT_NEED_MORE_DATA) {
                break;
            }
            handle_by_result(result, &frame, &frames_ok, &frames_err, &resync_count);
        }
    }
    fclose(file);

    // print stats
    printf("frame_ok: %zu, frames_err: %zu, resync_count: %zu\n", frames_ok, frames_err, resync_count);

    return 0;
}
