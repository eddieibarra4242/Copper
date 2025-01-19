#include "tokens.h"

#include <unity.h>

size_t get_token_list_length(Token *tokens) {
  size_t len = 0;
  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    len++;
  }

  return len;
}

void assert_coord_equal(Coord *expected, Coord *actual) {
  TEST_ASSERT_EQUAL(expected->line_number, actual->line_number);
  TEST_ASSERT_EQUAL(expected->column, actual->column);
}

void assert_span_equal(Span *expected, Span *actual) {
  assert_coord_equal(&expected->start, &actual->start);
  assert_coord_equal(&expected->end, &actual->end);
}

void assert_token_equal(Token *expected, Token *actual) {
  TEST_ASSERT_EQUAL(expected->kind, actual->kind);
  TEST_ASSERT_EQUAL(expected->length, actual->length);
  assert_span_equal(&expected->span, &actual->span);
  TEST_ASSERT_EQUAL_STRING(expected->data, actual->data);
}
