#include <scanner.h>
#include <stdbool.h>
#include <test_utils.h>
#include <unity.h>

void setUp(void) { reset_log_checks(); }

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

  // 5 ints + 1 EOF
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

  expect_error("Unexpected character ''' at 1:3, expected: [0-7]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_decimal_separator_failure(void) {
  const char *input = "12'";

  expect_error("Unexpected character ''' at 1:3, expected: [0-9]");
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
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_int_prefix(void) {
  const char *input = "0 0l 0L 0ll 0LL 0wb 0WB\n"
                      "0u 0ul 0uL 0ull 0uLL 0uwb 0uWB\n"
                      "0U 0Ul 0UL 0Ull 0ULL 0Uwb 0UWB\n"
                      "0u 0lu 0Lu 0llu 0LLu 0wbu 0WBu\n"
                      "0U 0lU 0LU 0llU 0LLU 0wbU 0WBU\n";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  // 35 ints + 1 EOF
  TEST_ASSERT_EQUAL(35 + 1, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF)
      break;

    TEST_ASSERT_EQUAL(CONSTANT, cur->kind);
  }
}

void test_wb_failure(void) {
  const char *input = "0wB";
  expect_error("Unexpected character 'B' at 1:3, expected: [b]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_ll_failure(void) {
  const char *input = "0Ll";
  expect_error("Unexpected character 'l' at 1:3, expected: [L]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_float(void) {
  const char *input = "0. .0dd 0x.abp-12DD";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  // 3 floats + 1 EOF
  TEST_ASSERT_EQUAL(3 + 1, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF)
      break;

    TEST_ASSERT_EQUAL(CONSTANT, cur->kind);
  }
}

// TODO: need more float tests.

void test_chars(void) {
  const char *input = "u8'\\uabcd' u'\\x12ef' L'\\777' '\\a' 'f' 'a' '$' '@'";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  // 8 chars + 1 EOF
  TEST_ASSERT_EQUAL(8 + 1, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF)
      break;

    TEST_ASSERT_EQUAL(CONSTANT, cur->kind);
  }
}

void test_char_newline_failure(void) {
  const char *input = "'\n'";
  expect_error("Unexpected character '.' at 1:2, expected: [a char]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

// TODO: need more char tests.

void test_punctuation(void) {
  const char *input = "[ ] ( ) { } . -> ++ --\n"
                      "& * + - ~ ! / % << >>\n"
                      "< > <= >= == != ^ | && ||\n"
                      "? : :: ; ... = *= /= %= +=\n"
                      "-= <<= >>= &= ^= |= , # ## <:\n"
                      ":> <% %> %: %:%:";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  // 55 punct + 1 EOF
  TEST_ASSERT_EQUAL(55 + 1, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF)
      break;

    TEST_ASSERT_EQUAL(PUNCT, cur->kind);
  }
}

void test_percent_failure(void) {
  const char *input = "%:%";
  expect_error("Unexpected character '.' at 1:4, expected: [:]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

void test_strings(void) {
  const char *input = "u8\"\\uabcdfdsafdsa\" u\"\\x12effdsafdsa\"\n"
                      "L\"\\777fdsafdsa\" \"\\aasd\" \"f\" \"affds\"\n"
                      "\"$as4523\" \"@12\"";
  Token *tokens = scan(input);

  size_t list_length = get_token_list_length(tokens);

  // 8 chars + 1 EOF
  TEST_ASSERT_EQUAL(8 + 1, list_length);

  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    if (cur->kind == EOF)
      break;

    TEST_ASSERT_EQUAL(STRING, cur->kind);
  }
}

void test_string_newline_failure(void) {
  const char *input = "\"\n\"";
  expect_error("Unexpected character '.' at 1:2, expected: [\"]");
  scan(input);
  TEST_FAIL_MESSAGE("No error detected!");
}

// TODO: need more string tests.
