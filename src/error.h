#pragma once

#include <stdlib.h>

#define WARNING_LEVEL 3
#define ERROR_LEVEL 2
#define CRITICAL_LEVEL 1

#define CRITICAL(section, msg) panic(CRITICAL_LEVEL, section, msg, __FILE__, __LINE__);

void panic(int level, const char *section, const char *msg, const char *file,
           int lineno);
