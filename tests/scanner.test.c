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
             "double else enum extern float for goto if\n"
             "inline int long register restrict return short\n"
             "signed sizeof static static_assert struct switch\n"
             "thread_local typedef typeof typeof_unqual union\n"
             "unsigned void volatile while _Atomic _BitInt\n"
             "_Complex _Decimal128 _Decimal32 _Decimal64 _Generic\n"
             "_Imaginary _Noreturn\n";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  // 51 keywords + 1 EOF
  TEST_ASSERT_EQUAL(51 + 1, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF)
      break;

    TEST_ASSERT_EQUAL(KEYWORD, cur->kind);
  }
}

void test_predefined_constants(void) {
  const char *input = "false nullptr true\n";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  // 3 predefined constants + 1 EOF
  TEST_ASSERT_EQUAL(3 + 1, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF)
      break;

    TEST_ASSERT_EQUAL(CONSTANT, cur->kind);
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

void test_constants(void) {
  const char *input = "0 10 0b11'11'11 0xabcdef0123456789 0'7'2'3";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  // 3 predefined constants + 1 EOF
  TEST_ASSERT_EQUAL(5 + 1, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF)
      break;

    TEST_ASSERT_EQUAL(CONSTANT, cur->kind);
  }
}

void test_binary_digit_failure(void) {
  const char *input = "0b12";

  expect_error("Unexpected character '2' at 1:4, expected: [0, 1]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_binary_separator_failure(void) {
  const char *input = "0b'11";

  expect_error("Unexpected character ''' at 1:3, expected: [0, 1]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_binary_separator_end_failure(void) {
  const char *input = "0b11'";

  expect_error("Unexpected character ''' at 1:5, expected: [0, 1]");
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);
  printf("%zu\n", list_length);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_octal_digit_failure(void) {
  const char *input = "08";

  expect_error("Unexpected character '8' at 1:2, expected: [0-7]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_octal_separator_failure(void) {
  const char *input = "07'";

  expect_error("Unexpected character ''' at 1:3, expected: [0, 1]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_decimal_digit_failure(void) {
  const char *input = "12a3";

  expect_error("Unexpected character 'a' at 1:3, expected: [0-9]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_decimal_separator_failure(void) {
  const char *input = "12'";

  expect_error("Unexpected character ''' at 1:3, expected: [0, 1]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_hex_digit_failure(void) {
  const char *input = "0x12g";

  expect_error("Unexpected character 'g' at 1:5, expected: [0-9, a-f, A-F]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_hex_separator_failure(void) {
  const char *input = "0x'a";

  expect_error("Unexpected character ''' at 1:3, expected: [0-9, a-f, A-F]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_hex_separator_end_failure(void) {
  const char *input = "0xa'";

  expect_error("Unexpected character ''' at 1:4, expected: [0-9, a-f, A-F]");
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);
  printf("%zu\n", list_length);
  TEST_FAIL_MESSAGE("No error detected!");
}
