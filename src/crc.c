#include <stddef.h>

#include "crc.h"

uint16_t crc_crc16_ccitt_false(const uint8_t *data, size_t len) {
  if (data == NULL || len == 0) {
    return 0xFFFF;
  }

  uint16_t result = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    result ^= (uint16_t)(data[i] << 8);
    for (size_t bit = 0; bit < 8; bit++) {
      if (result & 0x8000) {
        result = (uint16_t)((result << 1) ^ 0x1021);
      } else {
        result = (uint16_t)(result << 1);
      }
    }
  }
  return result;
}
