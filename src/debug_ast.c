#include "debug_ast.h"
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

void print_declaration(struct declaration *decl) {
  print("Declaration");

  stack++;

  if (decl->name)
    print_id(decl->name);

  stack--;
}

void print_declaration_list(struct declaration_list *list) {
  for (struct declaration *cur = list->head; cur != NULL; cur = cur->next) {
    print_declaration(cur);
  }
}

void print_translation_unit(struct translation_unit *unit) {
  print("Translation Unit");

  stack++;
  print_declaration_list(&unit->external_declarations);
  stack--;
}
