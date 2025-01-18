#include "log_utils.h"
#include <log.h>
#include <unity.h>
#include <stdarg.h>
#include <common.h>
#include <stdio.h>

const char *expected_message = NULL;
const char *expected_error_message = NULL;

void reset_log_checks(void) {
  expected_message = NULL;
  expected_error_message = NULL;
}

void expect_log(const char *message) { expected_message = message; }

void expect_error(const char *error_message) {
  expected_error_message = error_message;
}

void log_message(int level, const char *section, const char *msg,
                 const char *file, int lineno, ...) {
  UNUSED(section);
  UNUSED(file);
  char actual[1024];

  va_list args;
  va_start(args, lineno);
  vsnprintf(actual, 1024, msg, args);
  va_end(args);

  if (level > ERROR_LEVEL) {
    TEST_ASSERT_EQUAL_STRING(expected_message, actual);
  } else {
    TEST_ASSERT_EQUAL_STRING(expected_error_message, actual);
  }

  TEST_PASS();
}
