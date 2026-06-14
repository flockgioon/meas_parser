#include <stdint.h>

#include "big_endian.h"

uint16_t big_endian_uint16(const uint8_t *p) {
  return (uint16_t)(p[0] << 8 | p[1]);
}

uint32_t big_endian_uint32(const uint8_t *p) {
  return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
         ((uint32_t)p[2] << 8) | ((uint32_t)p[3]);
}

uint64_t big_endian_uint64(const uint8_t *p) {
  return ((uint64_t)p[0] << 56) | ((uint64_t)p[1] << 48) |
         ((uint64_t)p[2] << 40) | ((uint64_t)p[3] << 32) |
         ((uint64_t)p[4] << 24) | ((uint64_t)p[5] << 16) |
         ((uint64_t)p[6] << 8) | ((uint64_t)p[7]);
}

void big_endian_write_uint16(uint8_t *data, uint16_t v) {
  data[0] = (uint8_t)(v >> 8);
  data[1] = (uint8_t)v;
}

void big_endian_write_uint32(uint8_t *data, uint32_t v) {
  data[0] = (uint8_t)(v >> 24);
  data[1] = (uint8_t)(v >> 16);
  data[2] = (uint8_t)(v >> 8);
  data[3] = (uint8_t)v;
}
void big_endian_write_uint64(uint8_t *data, uint64_t v) {
  data[0] = (uint8_t)(v >> 56);
  data[1] = (uint8_t)(v >> 48);
  data[2] = (uint8_t)(v >> 40);
  data[3] = (uint8_t)(v >> 32);
  data[4] = (uint8_t)(v >> 24);
  data[5] = (uint8_t)(v >> 16);
  data[6] = (uint8_t)(v >> 8);
  data[7] = (uint8_t)v;
}
