#pragma once

#include <stdlib.h>

#define WARNING_LEVEL 3
#define ERROR_LEVEL 2
#define CRITICAL_LEVEL 1

#define ERROR(section, msg)                                                    \
  panic(ERROR_LEVEL, section, msg, __FILE__, __LINE__);

#define CRITICAL(section, msg)                                                 \
  panic(CRITICAL_LEVEL, section, msg, __FILE__, __LINE__);

#define ERRORV(section, msg, ...)                                              \
  panic(ERROR_LEVEL, section, msg, __FILE__, __LINE__, __VA_ARGS__);

#define CRITICALV(section, msg, ...)                                           \
  panic(CRITICAL_LEVEL, section, msg, __FILE__, __LINE__, __VA_ARGS__);

#define CHECK(result)                                                          \
  do {                                                                         \
    if (result == -1) {                                                        \
      panic(CRITICAL_LEVEL, "stdlib", NULL, __FILE__, __LINE__);               \
    }                                                                          \
  } while (0)

#define TRY(x)                                                                 \
  do {                                                                         \
    int result = (x);                                                          \
    if (result == -1) {                                                        \
      panic(CRITICAL_LEVEL, "stdlib", NULL, __FILE__, __LINE__);               \
    }                                                                          \
  } while (0)

void panic(int level, const char *section, const char *msg, const char *file,
           int lineno, ...);
