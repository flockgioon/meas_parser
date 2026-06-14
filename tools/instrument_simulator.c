#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "build_helper.h"
#include "mp_decode.h"
#include "portable.h"

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
  size_t frame_len = build_frame(MP_MSG_TYPE_MEASURE, 111, payload,
                                 (uint16_t)payload_len, frame);
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
  size_t frame_len = build_frame(MP_MSG_TYPE_MEASURE, 222, payload,
                                 (uint16_t)payload_len, frame);
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
  size_t frame_len = build_frame(MP_MSG_TYPE_MEASURE, 333, payload,
                                 (uint16_t)payload_len, frame);
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
  size_t frame_len = build_frame(MP_MSG_TYPE_MEASURE, 555, payload,
                                 (uint16_t)payload_len, frame);
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
