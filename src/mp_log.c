#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>

#include "mp_log.h"

// 預設使用 INFO level
static mp_log_level_t log_level = MP_LOG_LEVEL_INFO;

void mp_log_set_level(mp_log_level_t level) { log_level = level; }

void mp_log(mp_log_level_t level, const char *fmt, ...) {
  if (level < log_level) {
    return;
  }
  switch (level) {
  case MP_LOG_LEVEL_DEBUG:
    fprintf(stderr, "[DEBUG] ");
    break;
  case MP_LOG_LEVEL_INFO:
    fprintf(stderr, "[INFO] ");
    break;
  case MP_LOG_LEVEL_WARN:
    fprintf(stderr, "[WARN] ");
    break;
  case MP_LOG_LEVEL_ERROR:
    fprintf(stderr, "[ERROR] ");
    break;
  default:
    return;
  }

  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  fprintf(stderr, "\n");
}
