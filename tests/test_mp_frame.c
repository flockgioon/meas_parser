#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "big_endian.h"
#include "build_helper.h"
#include "mp_frame.h"
#include "protocol.h"

#define MP_RESULT_ERROR_FOR_TEST_INIT ((mp_result_t) - 999)

int main(void) {
  uint8_t buf[256];
  mp_frame_t f = {0};
  mp_result_t r = MP_RESULT_ERROR_FOR_TEST_INIT;

  /* 1. correct packet: 3 bytes payload */
  uint8_t payload[] = {0xDE, 0xAD, 0xBE};
  size_t total = build_frame(MP_MSG_TYPE_MEASURE, 42, payload, 3, buf);
  r = mp_frame_parse(buf, total, &f);
  assert(r == MP_RESULT_OK);
  assert(f.version == MP_VERSION);
  assert(f.msg_type == MP_MSG_TYPE_MEASURE);
  assert(f.seq == 42);
  assert(f.payload_len == 3);
  assert(f.payload[0] == 0xDE);
  assert(f.payload[1] == 0xAD);
  assert(f.payload[2] == 0xBE);

  /* 2. payload len = 0 */
  total = build_frame(MP_MSG_TYPE_ACK, 1, NULL, 0, buf);
  r = mp_frame_parse(buf, total, &f);
  assert(r == MP_RESULT_OK);
  assert(f.payload_len == 0);
  assert(f.msg_type == MP_MSG_TYPE_ACK);

  /* 3. bad sync bytes */
  build_frame(MP_MSG_TYPE_MEASURE, 1, payload, 3, buf);
  buf[0] = 0x00;
  r = mp_frame_parse(buf, total, &f);
  assert(r == MP_RESULT_ERROR_SYNC);

  /* 4. bad crc */
  total = build_frame(MP_MSG_TYPE_MEASURE, 1, payload, 3, buf);
  buf[9] ^= 0x01;
  r = mp_frame_parse(buf, total, &f);
  assert(r == MP_RESULT_ERROR_CRC);

  /* 5. payload_len > MP_MAX_PAYLOAD */
  build_frame(MP_MSG_TYPE_MEASURE, 1, payload, 3, buf);
  big_endian_write_uint16(buf + 6, MP_MAX_PAYLOAD + 1);
  r = mp_frame_parse(buf, 256, &f);
  assert(r == MP_RESULT_ERROR_LENGTH);

  /* 6. short buffer */
  total = build_frame(MP_MSG_TYPE_MEASURE, 1, payload, 3, buf);
  r = mp_frame_parse(buf, 5, &f);
  assert(r == MP_RESULT_NEED_MORE_DATA);

  /* 7. buffer len < total_len */
  r = mp_frame_parse(buf, total - 1, &f);
  assert(r == MP_RESULT_NEED_MORE_DATA);
}
