#include <stdio.h>

#include "assign.h"
#include "backend.h"
#include "common.h"
#include "debug_ast.h"
#include "debug_insn.h"
#include "emit.h"
#include "log.h"
#include "parser.h"
#include "preprocess.h"
#include "scanner.h"
#include "symbol.h"
#include "transforms.h"
#include "tree.h"

int main(int args, char **argv) {
  if (args < 2) {
    CRITICAL("cli", "No input file!");
  }

  Token *tokens = preprocess(argv[1]);

  if (tokens == NULL) {
    CRITICAL("lex", "Failed to scan file!");
  }

#ifndef NDEBUG
  for (Token *cur = tokens; cur != NULL; cur = cur->next) {
    DEBUG("%-15s \"%s\" [%zu:%zu, %zu:%zu]", kind_to_string(cur->kind),
          cur->data, cur->span.start.line_number, cur->span.start.column,
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

  // After semantic analysis
  transform_ast();
  assign_registers();

#ifndef NDEBUG
  print_ast();
#endif

  InstructionList *ir_insns = emit_intermediate_representation();
  if (ir_insns == NULL) {
    CRITICAL("emit", "Failed to emit intermediate representation!");
  }

#ifndef NDEBUG
  debug_insns(ir_insns);
#endif

  output_insns(ir_insns);

  destroy_instruction_list(ir_insns);
  free_generated_labels();
  destroy_ast();
  free_list(tokens);

  return 0;
}
