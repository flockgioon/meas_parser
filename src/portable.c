#ifndef _MSC_VER
#include <errno.h>
#endif

#include "portable.h"

int portable_fopen(FILE **file, const char *filepath, const char *mode) {
#ifdef _MSC_VER
  return (int)fopen_s(file, filepath, mode);
#else
  *file = fopen(filepath, mode);
  return *file == NULL ? errno : 0;
#endif
}