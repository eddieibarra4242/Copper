#include "debug_ast.h"
#include "ast.h"
#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void print_stmt(struct stmt *stmt);
void print_stmt_list(struct stmt_list *stmt_list);
void print_function(struct function *function);
void print_expression(struct expression *exp);

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

void print_ternary_exp(struct expression *exp) {
  print("Ternary Expression");

  stack++;
  print_expression(exp->exp._ternary.cond);
  print_expression(exp->exp._ternary.true_val);
  print_expression(exp->exp._ternary.false_val);
  stack--;
}

void print_binary_exp(struct expression *exp) {
  print("Binary Expression");

  stack++;
  print_token(exp->exp._binary.op);
  print_expression(exp->exp._binary.left);
  print_expression(exp->exp._binary.right);
  stack--;
}

void print_cast_exp(struct expression *exp) {
  print("Cast Expression");

  stack++;
  print_type(exp->exp._cast.type);
  print_expression(exp->exp._cast.child);
  stack--;
}

void print_prefix_exp(struct expression *exp) {
  print("Prefix Expression");

  stack++;
  print_token(exp->exp._prefix.op);
  print_expression(exp->exp._prefix.child);
  stack--;
}

void print_postfix_exp_index(struct expression *exp) {
  print("Postfix Expression (index)");

  stack++;
  print_expression(exp->exp._postfix.postfix.index);
  print_expression(exp->exp._postfix.child);
  stack--;
}

void print_postfix_exp_attr(struct expression *exp) {
  print("Postfix Expression (attr)");

  stack++;
  print_id(exp->exp._postfix.postfix.attribute);
  print_expression(exp->exp._postfix.child);
  stack--;
}

void print_postfix_exp(struct expression *exp) {
  switch (exp->exp._postfix.type) {
  case INDEX:
    print_postfix_exp_index(exp);
    return;
  case DOT:
  case ARROW:
    print_postfix_exp_attr(exp);
    return;
  default: break;
  }

  print("Postfix Expression");

  stack++;
  print("Type %d", exp->exp._postfix.type);
  print_expression(exp->exp._postfix.child);
  stack--;
}

void print_constant_exp(struct expression *exp) {
  print("Constant Expression");

  stack++;
  print_token(exp->exp._constant.token);
  stack--;
}

void print_id_exp(struct expression *exp) {
  print("ID Expression");

  stack++;
  print_id(exp->exp._id.id);
  stack--;
}

void print_expression(struct expression *exp) {
  switch (exp->type) {
  case IDENTIFIER_EXPR:
    print_id_exp(exp);
    break;
  case CONSTANT:
    print_constant_exp(exp);
    break;
  case POSTFIX:
    print_postfix_exp(exp);
    break;
  case PREFIX:
    print_prefix_exp(exp);
    break;
  case CAST:
    print_cast_exp(exp);
    break;
  case BINARY:
    print_binary_exp(exp);
    break;
  case TERNARY:
    print_ternary_exp(exp);
    break;
  }
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
  print_expression(if_stmt->condition);
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
    print_expression(return_stmt->opt_exp);
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

  print("Body");

  stack++;
  print_stmt_list(function->body);
  stack--;
  stack--;
}
