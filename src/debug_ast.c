#include "debug_ast.h"
#include "common.h"
#include "log.h"
#include "tree.h"

#include <stdarg.h>
#include <stdio.h>

void print_translation_unit(struct translation_unit *unit);

// Entry point
void print_ast() {
  AST tree = get_tree();
  print_translation_unit(tree);
}

size_t stack = 0;

char *get_prefix() {
  size_t length = 2 * stack + 1;
  char *prefix = malloc(length);

  if (prefix == NULL) {
    CRITICAL("print ast", "Out of memory!");
  }

  for (size_t i = 0; i < stack; i++) {
    prefix[2 * i] = '|';
    prefix[2 * i + 1] = ' ';
  }

  if (length >= 3) {
    prefix[length - 3] = '+';
    prefix[length - 2] = '-';
  }

  prefix[length - 1] = '\0';

  return prefix;
}

void print(const char *fmt, ...) {
  char fmtted_string[512];
  char *prefix = get_prefix();

  va_list args;
  va_start(args, fmt);
  vsnprintf(fmtted_string, 512, fmt, args);
  va_end(args);

  DEBUG("%s%s", prefix, fmtted_string);

  free(prefix);
}

void print_token(Token *token) { print("Token { %s }", token->data); }

void print_id(struct id *id) {
  print("Id");

  stack++;
  print_token(id->name);
  stack--;
}

void print_token_specifier(struct specifier *specifier) {
  print("Token specifier");

  stack++;
  print_token(specifier->_token);
  stack--;
}

void print_id_specifier(struct specifier *specifier) {
  print("Id specifier");

  stack++;
  print_id(specifier->_id);
  stack--;
}

void print_specifier(struct specifier *specifier) {
  switch (specifier->type) {
  case TOKEN:
    print_token_specifier(specifier);
    break;
  case ID_SPEC:
    print_id_specifier(specifier);
    break;
  }
}

void print_specifier_list(struct specifier_list *list) {
  for (struct specifier *cur = list->head; cur != NULL; cur = cur->next) {
    print_specifier(cur);
  }
}

void print_statement(struct statement *stmt);

void print_declaration(struct declaration *decl) {
  if (decl->body) {
    print("Function");
  } else {
    print("Declaration");
  }

  stack++;

  if (decl->specifiers)
    print_specifier_list(decl->specifiers);

  if (decl->name)
    print_id(decl->name);

  if (decl->body) {
    print_statement(decl->body);
  }

  stack--;
}

void print_declaration_list(struct declaration_list *list) {
  for (struct declaration *cur = list->head; cur != NULL; cur = cur->next) {
    print_declaration(cur);
  }
}

void print_break_stmt(struct statement *stmt) {
  UNUSED(stmt);

  print("Break");
}

void print_compound_stmt(struct statement *stmt) {
  print("Compound statement");

  stack++;
  for (struct statement *child = stmt->_compound.head; child != NULL;
       child = child->next) {
    print_statement(child);
  }
  stack--;
}

void print_continue_stmt(struct statement *stmt) {
  UNUSED(stmt);
  print("Continue");
}

void print_decl_stmt(struct statement *stmt) {
  print("Declaration statement");

  stack++;
  print_declaration(stmt->_decl);
  stack--;
}

void print_expr_stmt(struct statement *stmt) {
  UNUSED(stmt);
  print("Expression");
}

void print_for_stmt(struct statement *stmt) {
  print("For");

  stack++;

  if (stmt->_for.decl) {
    print_declaration(stmt->_for.decl);
  }

  if (stmt->_for.body) {
    print_statement(stmt->_for.body);
  }

  stack--;
}

void print_goto_stmt(struct statement *stmt) {
  print("Goto");

  stack++;
  print_id(stmt->_goto);
  stack--;
}

void print_if_stmt(struct statement *stmt) {
  print("If");

  stack++;

  if (stmt->_if.body) {
    print_statement(stmt->_if.body);
  }

  if (stmt->_if.else_body) {
    print_statement(stmt->_if.else_body);
  }

  stack--;
}

void print_label_stmt(struct statement *stmt) {
  print("Label");

  stack++;
  print_id(stmt->_label.name);
  stack--;
}

void print_return_stmt(struct statement *stmt) {
  UNUSED(stmt);
  print("Return");
}

void print_switch_stmt(struct statement *stmt) {
  print("Switch");

  stack++;

  if (stmt->_switch.body) {
    print_statement(stmt->_switch.body);
  }

  stack--;
}

void print_switch_label_stmt(struct statement *stmt) {
  UNUSED(stmt);
  print("Switch label");
}

void print_while_stmt(struct statement *stmt) {
  if (stmt->_while.should_check_condition_first) {
    print("While");
  } else {
    print("Do while");
  }

  stack++;
  if (stmt->_while.body) {
    print_statement(stmt->_while.body);
  }

  stack--;
}

void print_statement(struct statement *stmt) {
  switch (stmt->type) {
  case BREAK:
    print_break_stmt(stmt);
    break;
  case COMPOUND:
    print_compound_stmt(stmt);
    break;
  case CONTINUE:
    print_continue_stmt(stmt);
    break;
  case DECL:
    print_decl_stmt(stmt);
    break;
  case EXPR:
    print_expr_stmt(stmt);
    break;
  case FOR:
    print_for_stmt(stmt);
    break;
  case GOTO:
    print_goto_stmt(stmt);
    break;
  case IF:
    print_if_stmt(stmt);
    break;
  case LABEL:
    print_label_stmt(stmt);
    break;
  case RETURN:
    print_return_stmt(stmt);
    break;
  case SWITCH:
    print_switch_stmt(stmt);
    break;
  case SWITCH_LABEL:
    print_switch_label_stmt(stmt);
    break;
  case WHILE:
    print_while_stmt(stmt);
    break;
  }
}

void print_translation_unit(struct translation_unit *unit) {
  print("Translation Unit");

  stack++;
  print_declaration_list(&unit->external_declarations);
  stack--;
}
