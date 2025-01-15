#include "log.h"
#include "common.h"

#include <stdarg.h>
#include <stdio.h>

void log_message(int level, const char *section, const char *msg,
                 const char *file, int lineno, ...) {
  FILE *output_file = level < WARNING_LEVEL ? stderr : stdout;

#ifndef NDEBUG
  fprintf(output_file, "[%s:%d] ", file, lineno);
#else
  UNUSED(file);
  UNUSED(lineno);
#endif

  va_list args;
  va_start(args, lineno);

  if (msg) {
    fprintf(output_file, "[%s] ", section);
    vfprintf(output_file, msg, args);
    fprintf(output_file, "\n");
  } else {
    fprintf(output_file, "[%s] ", section);
    perror("stdlib error");
  }

  va_end(args);

  if (level < WARNING_LEVEL)
    exit(level);
}
