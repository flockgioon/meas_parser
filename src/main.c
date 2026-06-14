#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "mp_decode.h"
#include "mp_frame.h"
#include "mp_log.h"
#include "mp_parser.h"
#include "protocol.h"
#include "portable.h"

#define MP_RESULT_INIT 999

static const char *mp_unit_to_string(mp_unit_t unit);
static void handle_by_result(mp_result_t result, mp_frame_t *frame, size_t *frames_ok, size_t *frames_err, size_t *resync_count);
static void run_parser(mp_parser_t *parser, uint8_t *buf, size_t n, size_t *frames_ok, size_t *frames_err, size_t *resync_count);
static int process_file(const char *filename);
static int process_tcp(const char *host, uint16_t port);

int main(int argc, char *argv[]) {
    // usage:
    //     ./meas_parser <file.bin>
    //     ./meas_parser --tcp <host> <port>

    if (argc < 2) {
        fprintf(stderr, "usage:\n");
        fprintf(stderr, "    %s <file.bin>\n", argv[0]);
        fprintf(stderr, "    %s --tcp <host> <port>\n", argv[0]);
        return 1;
    }

    // set log level
    mp_log_set_level(MP_LOG_LEVEL_DEBUG);

    int is_tcp_mode = (argc >= 4 && strcmp(argv[1], "--tcp") == 0);
    if (is_tcp_mode) {
        const char *host = argv[2];
        char *end = NULL;
        uint16_t port = (uint16_t)strtol(argv[3], &end, 10);
        if (*end != '\0') {
            fprintf(stderr, "port should be a number");
            return 1;   
        }
        return process_tcp(host, port);
    }
    const char *filename = argv[1];
    return process_file(filename);
}

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

static void run_parser(mp_parser_t *parser, uint8_t *buf, size_t n, size_t *frames_ok, size_t *frames_err, size_t *resync_count) {
    mp_parser_feed(parser, buf, n);

    mp_frame_t frame = {0};
    mp_result_t result = MP_RESULT_INIT;

    while (1) {
        result = mp_parser_consume(parser, &frame);
        if (result == MP_RESULT_NEED_MORE_DATA) {
            break;
        }
        handle_by_result(result, &frame, frames_ok, frames_err, resync_count);
    }
}

static int process_file(const char *filename) {
    // read file
    FILE *file = NULL;
    int err = portable_fopen(&file, filename, "rb");
    if (err != 0) {
        fprintf(stderr, "failed to fopen_s: %d\n", err);
        return 1;
    }

    // mp_parser
    mp_parser_t parser = {0};

    // for stats
    size_t frames_ok = 0;
    size_t frames_err = 0;
    size_t resync_count = 0;

    // read loop
    uint8_t buf[256] = {0};
    size_t n = 0;
    while (1) {
        n = fread(buf, 1, sizeof(buf), file);
        if (n <= 0) {
            break;
        }
        run_parser(&parser, buf, n, &frames_ok, &frames_err, &resync_count);
    }
    fclose(file);

    // print stats
    printf("frame_ok: %zu, frames_err: %zu, resync_count: %zu\n", frames_ok, frames_err, resync_count);
    return 0;
}

static int process_tcp(const char *host, uint16_t port) {
    // socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("failed to socket");
        return 1;
    }

    //  bind
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host);

    // connect
    int ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0) {
        perror("failed to connect");
        return 1;
    }
    printf("connected to %s:%u\n", host, port);

    // mp_parser
    mp_parser_t parser = {0};

    // for stats
    size_t frames_ok = 0;
    size_t frames_err = 0;
    size_t resync_count = 0;

    // recv loop
    ssize_t n = 0;
    uint8_t buf[256] = {0};

    while (1) {
        n = recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) {
            break;
        }
        run_parser(&parser, buf, (size_t)n, &frames_ok, &frames_err, &resync_count);
    }

    // close
    close(fd);
    return 0;
}