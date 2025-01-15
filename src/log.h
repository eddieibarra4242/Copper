#pragma once

#include <stdlib.h>

#define DEBUG_LEVEL 10
#define WARNING_LEVEL 3
#define ERROR_LEVEL 2
#define CRITICAL_LEVEL 1

#define DEBUG(msg, ...)                                                        \
  log_message(DEBUG_LEVEL, "debug", msg, __FILE__, __LINE__, __VA_ARGS__)

#define ERROR(section, msg)                                                    \
  log_message(ERROR_LEVEL, section, msg, __FILE__, __LINE__);

#define CRITICAL(section, msg)                                                 \
  log_message(CRITICAL_LEVEL, section, msg, __FILE__, __LINE__);

#define ERRORV(section, msg, ...)                                              \
  log_message(ERROR_LEVEL, section, msg, __FILE__, __LINE__, __VA_ARGS__);

#define CRITICALV(section, msg, ...)                                           \
  log_message(CRITICAL_LEVEL, section, msg, __FILE__, __LINE__, __VA_ARGS__);

#define CHECK(result)                                                          \
  do {                                                                         \
    if (result == -1) {                                                        \
      log_message(CRITICAL_LEVEL, "stdlib", NULL, __FILE__, __LINE__);         \
    }                                                                          \
  } while (0)

#define TRY(x)                                                                 \
  do {                                                                         \
    int result = (x);                                                          \
    if (result == -1) {                                                        \
      log_message(CRITICAL_LEVEL, "stdlib", NULL, __FILE__, __LINE__);         \
    }                                                                          \
  } while (0)

void log_message(int level, const char *section, const char *msg, const char *file,
           int lineno, ...);
