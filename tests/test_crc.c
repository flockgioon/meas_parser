#include <assert.h>
#include <stddef.h>

#include "crc.h"

int main(void) {

  const uint8_t data1[] = "123456789";
  assert(crc_crc16_ccitt_false(data1, 9) == 0x29B1);

  const uint8_t data2[] = "48763";
  assert(crc_crc16_ccitt_false(data2, 5) == 0xD4F8);

  const uint8_t data3[] = "114514";
  assert(crc_crc16_ccitt_false(data3, 6) == 0x4ABD);

  assert(crc_crc16_ccitt_false(NULL, 0) == 0xFFFF);
  assert(crc_crc16_ccitt_false(NULL, 1) == 0xFFFF);

  return 0;
}
