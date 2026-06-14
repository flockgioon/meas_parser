#include <stddef.h>

#include "big_endian.h"
#include "mp_decode.h"
#include "protocol.h"

mp_result_t mp_decode_measurement(mp_frame_t *f, mp_measurement_t *out) {
  if (f == NULL || out == NULL) {
    return MP_RESULT_ERROR_INVALID_ARGS;
  }

  if (f->msg_type != MP_MSG_TYPE_MEASURE) {
    return MP_RESULT_ERROR_PAYLOAD;
  }

  if (f->payload_len < 5) {
    return MP_RESULT_ERROR_PAYLOAD;
  }

  uint32_t timestamp = big_endian_uint32(f->payload);
  uint8_t channel_count = f->payload[4];
  if (channel_count > MP_MAX_CHANNELS ||
      5 + 6 * channel_count != f->payload_len) {
    return MP_RESULT_ERROR_PAYLOAD;
  }

  out->timestamp_ms = timestamp;
  out->channel_count = channel_count;
  for (int i = 0; i < channel_count; i++) {
    int base = 5 + 6 * i;
    out->readings[i] = (mp_reading_t){
        .channel_id = f->payload[base],
        .unit = f->payload[base + 1],
        .value_milli = (int32_t)big_endian_uint32(&f->payload[base + 2]),
    };
  }
  return MP_RESULT_OK;
}
