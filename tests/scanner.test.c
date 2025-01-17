#include <unity.h>
#include <stdbool.h>
#include <scanner.h>

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

size_t get_token_list_length(Token *tokens) {
  size_t len = 0;
  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    len++;
  }

  return len;
}

void assert_token_equal(Token *expected, Token *actual) {
  TEST_ASSERT_EQUAL(expected->kind, actual->kind);
  TEST_ASSERT_EQUAL(expected->start, actual->start);
  TEST_ASSERT_EQUAL(expected->end, actual->end);
  TEST_ASSERT_EQUAL(expected->line_number, actual->line_number);
  TEST_ASSERT_EQUAL(expected->column, actual->column);
  TEST_ASSERT_EQUAL_STRING(expected->data, actual->data);
}

void test_simple_program(void) {
  const char *input = "int main() {}";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  Token *cur = tokens;
  TEST_ASSERT_EQUAL(7, list_length);

  Token k;
  k.column = 1;
  k.data = "int";
  k.end = 3;
  k.kind = KEYWORD;
  k.line_number = 1;
  k.next = NULL;
  k.start = 0;

  assert_token_equal(&k, cur);

  free_list(tokens);
}

void test_identifier(void) {
  const char *input = "a bcd __this_is_an_identifier__ he110";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  TEST_ASSERT_EQUAL(5, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF) break;

    TEST_ASSERT_EQUAL(IDENTIFIER, cur->kind);
  }

  free_list(tokens);
}

void test_non_identifier(void) {
  const char *input = "9hello"; // This is two different tokens
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  TEST_ASSERT_EQUAL(3, list_length);
  TEST_ASSERT_NOT_EQUAL(IDENTIFIER, tokens->kind);

  free_list(tokens);
}

void test_eof(void) {
  const char *input = "";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  TEST_ASSERT_EQUAL(1, list_length);
  TEST_ASSERT_EQUAL(EOF, tokens->kind);

  free_list(tokens);
}
