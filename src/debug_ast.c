#include "debug_ast.h"
#include "ast.h"
#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void print_stmt(struct stmt *stmt);
void print_stmt_list(struct stmt_list *stmt_list);
void print_function(struct function *function);

// Entry point
void print_ast() {
  AST tree = get_tree();
  print_function(tree);
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

  prefix[length - 3] = '+';
  prefix[length - 2] = '-';
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
}

void print_token(Token *token) { print("Token { %s }", token->data); }

void print_type(struct type *type) {
  print("Type");

  stack++;
  print_token(type->token);
  stack--;
}

void print_id(struct id *id) {
  print("Id");

  stack++;
  print_token(id->token);
  stack--;
}

void print_exp(struct exp *exp) {
  print("Expression");

  stack++;
  print_token(exp->token);
  stack--;
}

void print_else_stmt(struct else_stmt *else_stmt) {
  print("Else");

  stack++;

  print("Body");

  stack++;
  print_stmt(else_stmt->body);
  stack--;
  stack--;
}

void print_if_stmt(struct if_stmt *if_stmt) {
  print("If");

  stack++;

  print("Condition");

  stack++;
  print_exp(if_stmt->condition);
  stack--;

  print("Body");

  stack++;
  print_stmt(if_stmt->body);
  stack--;

  if (if_stmt->opt_else)
    print_else_stmt(if_stmt->opt_else);

  stack--;
}

void print_return_stmt(struct return_stmt *return_stmt) {
  print("Return");

  stack++;
  if (return_stmt->opt_exp)
    print_exp(return_stmt->opt_exp);
  stack--;
}

void print_compound_stmt(struct compound_stmt *compound_stmt) {
  print("Compound statement");

  stack++;
  print_stmt_list(compound_stmt->list);
  stack--;
}

void print_stmt(struct stmt *stmt) {
  switch (stmt->type) {
  case IF:
    print_if_stmt(&stmt->stmt._if);
    break;
  case RETURN:
    print_return_stmt(&stmt->stmt._return);
    break;
  case COMPOUND:
    print_compound_stmt(&stmt->stmt._compound);
    break;
  }
}

void print_stmt_list(struct stmt_list *stmt_list) {
  for (struct stmt *cur = stmt_list->head; cur != NULL;) {
    struct stmt *next = cur->next;
    print_stmt(cur);
    cur = next;
  }
}

void print_function(struct function *function) {
  print("Function");

  stack++;
  print_type(function->return_type);
  print_id(function->name);
  print_stmt_list(function->body);
  stack--;
}
