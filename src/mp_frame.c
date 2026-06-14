#include <stdint.h>
#include <string.h>

#include "big_endian.h"
#include "crc.h"
#include "mp_frame.h"
#include "protocol.h"

mp_result_t mp_frame_parse(const uint8_t *buf, size_t buf_len, mp_frame_t *out) {
    // minimum buffer len check
    // minimum buffer: header(8) + payload(0) + crc(2) = 10
    if (buf_len < 10) {
        return MP_RESULT_NEED_MORE_DATA;
    }

    // sync bytes check
    if (buf[0] != MP_SYNC0 || buf[1] != MP_SYNC1) {
        return MP_RESULT_ERROR_SYNC;
    }

    // header parse
    uint8_t version = buf[2];
    uint8_t msg_type = buf[3];
    uint16_t seq = big_endian_uint16(&buf[4]);
    uint16_t payload_len = big_endian_uint16(&buf[6]);
    size_t total_len = MP_HEADER_LEN + payload_len + MP_CRC_LEN;

    // version check
    if (version != MP_VERSION) {
        return MP_RESULT_ERROR_VERSION;
    }

    // payload len check
    if (payload_len > MP_MAX_PAYLOAD) {
        return MP_RESULT_ERROR_LENGTH;
    }

    // buffer len check
    if (buf_len < total_len) {
        return MP_RESULT_NEED_MORE_DATA;
    }

    // crc check    
    out->seq = seq; // write earlier

    uint16_t crc = big_endian_uint16(&buf[8 + payload_len]);
    uint16_t crc_prime = crc_crc16_ccitt_false(&buf[2], 6 + payload_len);
    if (crc != crc_prime) {
        return MP_RESULT_ERROR_CRC;
    }

    // ok
    out->version = version;
    out->msg_type = msg_type;
    // out->seq = seq;
    out->payload_len = payload_len;
    out->crc = crc;
    memcpy(out->payload, &buf[8], payload_len);

    return MP_RESULT_OK;
}
