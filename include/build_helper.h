#include <stdint.h>
#include <stddef.h>

#include "protocol.h"
#include "mp_decode.h"

/*
 * return: buffer total len
 */
size_t build_frame(
    mp_msg_type_t msg_type,
    uint16_t seq,
    const uint8_t *payload,
    uint16_t payload_len,
    uint8_t *out_buf
);

size_t build_measure_payload(
    uint32_t timestamp,
    uint8_t channel_count,
    const mp_reading_t *readings,
    uint8_t *out
);

void make_measure_frame(const uint8_t *payload, uint16_t payload_len, mp_frame_t *out);