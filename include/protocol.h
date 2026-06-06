#pragma once

#include <stdint.h>

#define MP_SYNC0 0xAAu
#define MP_SYNC1 0x55u
#define MP_VERSION 1u
#define MP_HEADER_LEN 8u
#define MP_CRC_LEN 2u
#define MP_MAX_PAYLOAD 1024u

typedef enum {
    MP_MSG_TYPE_MEASURE = 0x01,
    MP_MSG_TYPE_STATUS = 0x02,
    MP_MSG_TYPE_ACK = 0x03,
    MP_MSG_TYPE_ERROR = 0x04,
} mp_msg_type_t;

typedef enum {
    MP_RESULT_OK = 0,
    MP_NEED_MORE_DATA,
    MP_RESYNC,
    MP_ERROR_SYNC,
    MP_ERROR_VERSION,
    MP_ERROR_LENGTH,
    MP_ERROR_CRC,
    MP_ERROR_PAYLOAD,
    MP_ERROR_IO,
} mp_result_t;
