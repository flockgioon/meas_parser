#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "build_helper.h"
#include "mp_decode.h"
#include "mp_log.h"
#include "mp_socket.h"
#include "protocol.h"

#define PORT 9000

static inline void send_with_log(mp_socket_t fd, uint8_t *buf, size_t buf_len, char *send_name) {
    ssize_t n = send((mp_socket_t)fd, (char *)buf, (int)buf_len, 0);
    if (n < 0) {
        mp_log(MP_LOG_LEVEL_ERROR, "%s fail", send_name);
    } else {
        mp_log(MP_LOG_LEVEL_INFO, "%s success", send_name);
    }
}

static void send_junk(mp_socket_t fd) {
    uint8_t junk[] = {0xAA, 0xBB, 0xCC, 0xDD};
    send_with_log(fd, junk, sizeof(junk), "send_junk");
}

static void send_valid_measure_frame_with_2_channel(mp_socket_t fd) {
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
    send_with_log(fd, frame, frame_len, "send_valid_measure_frame_with_2_channel");
}

static void send_valid_measure_frame_with_1_channel(mp_socket_t fd) {
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
    send_with_log(fd, frame, frame_len, "send_valid_measure_frame_with_1_channel");
}

static void send_bad_crc(mp_socket_t fd) {
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
    send_with_log(fd, frame, frame_len, "send_bad_crc");
}

static void send_ack_frame(mp_socket_t fd) {
    uint8_t frame[256] = {0};
    size_t frame_len = build_frame(MP_MSG_TYPE_ACK, 444, NULL, 0, frame);
    send_with_log(fd, frame, frame_len, "send_ack_frame");
}

static void send_valid_measure_frame_with_3_channel(mp_socket_t fd) {
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
    send_with_log(fd, frame, frame_len, "send_valid_measure_frame_with_3_channel");
}

int main(void) {
    if (mp_net_init() != 0) {
        fprintf(stderr, "network init failed\n");
        return 1;
    }

    // socket
    mp_socket_t server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == MP_INVALID_SOCKET) {
        mp_print_socket_error("failed to socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    // bind
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        mp_print_socket_error("failed to bind");
        mp_socket_close(server_fd);
        return 1;
    }

    // listen
    listen(server_fd, 1);
    printf("listening on port %d, waiting for client...\n", PORT);

    // accept
    mp_socket_t client_fd = accept(server_fd, NULL, NULL);
    if (client_fd == MP_INVALID_SOCKET) {
        mp_print_socket_error("failed to accept");
        mp_socket_close(server_fd);
        return 1;
    }
    printf("client connected\n");

    // send
    void (*send_funcs[])(mp_socket_t) = {
        send_junk,
        send_valid_measure_frame_with_2_channel,
        send_valid_measure_frame_with_1_channel,
        send_bad_crc,
        send_ack_frame,
        send_valid_measure_frame_with_3_channel,
    };
    size_t funcs_num = sizeof(send_funcs) / sizeof(send_funcs[0]);

    for (size_t i = 0; i < funcs_num; i++) {
        send_funcs[i](client_fd);
        mp_sleep(1);
    }

    mp_socket_close(server_fd);
    mp_socket_close(client_fd);

    mp_net_cleanup();

    return 0;
}
