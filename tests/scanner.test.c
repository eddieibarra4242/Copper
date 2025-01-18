#include <unity.h>
#include <stdbool.h>
#include <scanner.h>
#include <test_utils.h>

void setUp(void) {
  reset_log_checks();
}

void tearDown(void) {
  // clean stuff up here
}

void test_simple_program(void) {
  const char *input = "int main() {}";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  Token *cur = tokens;
  TEST_ASSERT_EQUAL(7, list_length);

  Token k;
  k.kind = KEYWORD;
  k.length = 3;
  k.data = "int";

  k.span.start.line_number = 1;
  k.span.start.column = 1;
  k.span.end.line_number = 1;
  k.span.end.column = 4;

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
