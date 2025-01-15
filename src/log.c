#include "log.h"
#include "common.h"

#include <stdarg.h>
#include <stdio.h>

#define CYAN ";36"
#define BLUE ";34"
#define YELLOW ";33"
#define RED ";31"
#define RESET ""

#ifndef NO_COLORS
#define C(file, color) fprintf(file, "\033[0" color "m")
#else
#define C(file, color)
#endif

void log_message(int level, const char *section, const char *msg,
                 const char *file, int lineno, ...) {
  FILE *output_file = level < WARNING_LEVEL ? stderr : stdout;

  if (level == DEBUG_LEVEL) {
    C(output_file, CYAN);
  } else if (level == WARNING_LEVEL) {
    C(output_file, YELLOW);
  } else if (level == ERROR_LEVEL || level == CRITICAL_LEVEL) {
    C(output_file, RED);
  } else {
    C(output_file, BLUE);
  }

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
    C(output_file, RESET);
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
