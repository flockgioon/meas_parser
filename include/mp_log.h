#pragma once

typedef enum {
  MP_LOG_LEVEL_DEBUG,
  MP_LOG_LEVEL_INFO,
  MP_LOG_LEVEL_WARN,
  MP_LOG_LEVEL_ERROR,
} mp_log_level_t;

void mp_log_set_level(mp_log_level_t level);
void mp_log(mp_log_level_t level, const char *fmt, ...);
