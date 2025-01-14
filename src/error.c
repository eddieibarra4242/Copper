#include "error.h"
#include "common.h"

#include <stdio.h>

void panic(int level, const char *section, const char *msg, const char *file,
           int lineno) {
  FILE *output_file = level < WARNING_LEVEL ? stderr : stdout;

#ifndef NDEBUG
  fprintf(output_file, "[%s:%d] ", file, lineno);
#else
  UNUSED(file);
  UNUSED(lineno);
#endif

  if (msg) {
    fprintf(output_file, "[%s] %s\n", section, msg);
  } else {
    fprintf(output_file, "[%s] ", section);
    perror("stdlib error");
  }

  if (level < WARNING_LEVEL)
    exit(level);
}
