#include <stdio.h>

#include "common.h"
#include "debug_ast.h"
#include "log.h"
#include "parser.h"
#include "scanner.h"
#include "tree.h"
#include "assign.h"
#include "symbol.h"

int main(int args, char **argv) {
  if (args < 2) {
    CRITICAL("cli", "No input file!");
  }

  FILE *input = fopen(argv[1], "r");

  if (!input) {
    CRITICAL("cli", "Failed to open input file!");
  }

  TRY(fseek(input, 0, SEEK_END));
  long file_size = ftell(input);

  CHECK(file_size);

  TRY(fseek(input, 0, SEEK_SET));

  char *file = malloc(file_size + 1);
  if (!file) {
    ERROR("file", "Out of memory!");
  }

  size_t read_len = fread(file, sizeof(char), file_size, input);

  if (read_len != (size_t)file_size) {
    // FIXME: do not fail here...
    ERROR("read", "Failed to read whole file!");
  }

  file[file_size] = '\0';

  TRY(fclose(input));

  Token *tokens = scan(file);

  free(file);

  if (tokens == NULL) {
    CRITICAL("lex", "Failed to scan file!");
  }

#ifndef NDEBUG
  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    DEBUG("token [%d, %s, %zu:%zu, %zu:%zu]", cur->kind, cur->data,
          cur->span.start.line_number, cur->span.start.column,
          cur->span.end.line_number, cur->span.end.column);
  }
#endif

  init_parser(tokens);
  int result = yyparse();

  if (result != 0) {
    CRITICAL("parser", "Failed to parse file!");
  }

  free_type_alias_memory();
  free_unused_parse_branches();

  link_symbols();
  assign_registers();

#ifndef NDEBUG
  print_ast();
#endif

  destroy_ast();
  free_list(tokens);

  return 0;
}
