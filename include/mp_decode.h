#pragma once

#include <stdint.h>

#include "mp_frame.h"
#include "protocol.h"

#define MP_MAX_CHANNELS 64

/* payload buffer structure
 * offset   size           field
 *      0      4    TIMESTAMP_MS
 *      4      1    CHANNEL_COUNT
 *  5+0*6      6    READING_0: channel_id(1) + unit(1) + value_mili(4)
 *  5+1*6     11    READING_1: ...
 *     ...
 */

typedef uint8_t mp_unit_t;

enum {
    MP_UNIT_VOLT = 0,
    MP_UNIT_AMP = 1,
    MP_UNIT_CELSIUS = 2,
};

typedef struct {
    uint8_t channel_id;
    mp_unit_t unit;
    int32_t value_milli;
} mp_reading_t;

typedef struct {
    uint32_t timestamp_ms;
    uint8_t channel_count;
    mp_reading_t readings[MP_MAX_CHANNELS];
} mp_measurement_t;

mp_result_t mp_decode_measurement(mp_frame_t *f, mp_measurement_t *out);
