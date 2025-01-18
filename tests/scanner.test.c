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

void test_eof(void) {
  const char *input = "";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  TEST_ASSERT_EQUAL(1, list_length);
  TEST_ASSERT_EQUAL(EOF, tokens->kind);

  free_list(tokens);
}

void test_keywords(void) {
  const char *input = "alignas alignof auto bool break case\n"
             "char const constexpr continue default do\n"
             "double else enum extern false float for\n"
             "goto if inline int long nullptr register\n"
             "restrict return short signed sizeof static\n"
             "static_assert struct switch thread_local true\n"
             "typedef typeof typeof_unqual union unsigned void\n"
             "volatile while _Atomic _BitInt _Complex\n"
             "_Decimal128 _Decimal32 _Decimal64 _Generic\n"
             "_Imaginary _Noreturn";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  // 54 keywords + 1 EOF
  TEST_ASSERT_EQUAL(54 + 1, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF)
      break;

    TEST_ASSERT_EQUAL(KEYWORD, cur->kind);
  }
}

void test_identifier(void) {
  const char *input = "a bcd __this_is_an_identifier__ he110";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  TEST_ASSERT_EQUAL(5, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF)
      break;

    TEST_ASSERT_EQUAL(IDENTIFIER, cur->kind);
  }

  free_list(tokens);
}

void test_regular_comment(void) {
  const char *input = "// this is a regular comment";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  TEST_ASSERT_EQUAL(1, list_length);

  free_list(tokens);
}

void test_inline_comment(void) {
  const char *input = "int hello /* This is\n"
  "a comment */ hi how are you?";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  TEST_ASSERT_EQUAL(8, list_length);

  free_list(tokens);
}
