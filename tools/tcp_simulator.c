#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "mp_decode.h"
#include "mp_log.h"
#include "protocol.h"
#include "big_endian.h"
#include "crc.h"


#define PORT 9000


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

static inline void send_with_log(int fd, uint8_t *buf, size_t buf_len, char * send_name) {
    ssize_t n = send(fd, buf, buf_len, 0); 
    if (n < 0) { 
        mp_log(MP_LOG_LEVEL_ERROR, "%s fail", send_name); 
    } else { 
        mp_log(MP_LOG_LEVEL_INFO, "%s success", send_name); 
    }
}

static void send_junk(int fd) {
    uint8_t junk[] = {0xAA, 0xBB, 0xCC, 0xDD};
    send_with_log(fd, junk, sizeof(junk), "send_junk");
}

static void send_valid_measure_frame_with_2_channel(int fd) {
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

static void send_valid_measure_frame_with_1_channel(int fd) {
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

static void send_bad_crc(int fd) {
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

static void send_ack_frame(int fd) {
    uint8_t frame[256] = {0};
    size_t frame_len = build_frame(MP_MSG_TYPE_ACK, 444, NULL, 0, frame);
    send_with_log(fd, frame, frame_len, "send_ack_frame");
}

static void send_valid_measure_frame_with_3_channel(int fd) {
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
    
    // socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("failed to socket");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // bind
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("failed to bind");
        return 1;
    }

    // listen
    listen(server_fd, 1);
    printf("listening on port %d, waiting for client...\n", PORT);

    // accept
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("failed to accept");
        close(server_fd);
        return 1;
    }
    printf("client connected\n");

    // send
    void (*send_funcs[])(int) = {
        send_junk,
        send_valid_measure_frame_with_2_channel,
        send_valid_measure_frame_with_1_channel,
        send_bad_crc,
        send_ack_frame,
        send_valid_measure_frame_with_3_channel,
    };
    size_t funcs_num = sizeof(send_funcs)/sizeof(send_funcs[0]);

    for (size_t i = 0; i < funcs_num; i++) {
        send_funcs[i](client_fd);
        sleep(1);
    }
    
    return 0;
}