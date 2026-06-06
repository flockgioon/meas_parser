#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "crc.h"

int main(void) {
    uint8_t data[] = {0xAA, 0x55, 0x01, 0x01, 0x00, 0x2A, 0x00, 0x00};
    uint16_t crc_1 = crc_crc16_ccitt_false(data, 8);

    // data[5] = 0x01;
    uint16_t crc_2 = crc_crc16_ccitt_false(data, 8);
    printf("crc_1: %u, crc_2: %u\n", crc_1, crc_2);

    return 0;
}
