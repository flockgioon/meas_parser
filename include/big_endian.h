#pragma once

#include <stdint.h>

uint16_t big_endian_uint16(const uint8_t *data);
uint32_t big_endian_uint32(const uint8_t *data);
uint64_t big_endian_uint64(const uint8_t *data);

void big_endian_write_uint16(const uint8_t *data, uint16_t v);
void big_endian_write_uint32(const uint8_t *data, uint32_t v);
void big_endian_write_uint64(const uint8_t *data, uint64_t v);
