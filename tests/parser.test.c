#include <test_utils.h>
#include <unity.h>
#include <scanner.h>
#include <parser.h>

void setUp(void) { reset_log_checks(); }
void tearDown(void) { }

void test_simple_program(void) {
  const char* input = "int main(){}";
  Token *tokens = scan(input);

  init_parser(tokens);
  int result = yyparse();

  TEST_ASSERT_EQUAL(0, result);
}
