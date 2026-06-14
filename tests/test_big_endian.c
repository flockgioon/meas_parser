#include <assert.h>

#include "big_endian.h"

int main(void) {
  uint8_t a[] = {0x12, 0x34};
  assert(big_endian_uint16(a) == 0x1234);

  uint8_t b[] = {0xAA, 0x55};
  assert(big_endian_uint16(b) == 0xAA55);

  uint8_t c[] = {0x12, 0x34, 0x56, 0x78};
  assert(big_endian_uint32(c) == 0x12345678);

  uint8_t d[] = {0xFF, 0x00, 0x00, 0x00};
  assert(big_endian_uint32(d) == 0xFF000000);

  uint8_t e[] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
  assert(big_endian_uint64(e) == 0x1122334455667788);
  return 0;
}
