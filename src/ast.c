#include "ast.h"
#include "log.h"

#include <stdlib.h>

struct function *root = NULL;

AST get_tree() { return root; }

void *allocate_or_error(size_t size) {
  void *result = malloc(size);

  if (result == NULL) {
    CRITICAL("ast", "Out of memory!");
  }

  return result;
}

struct type *create_type(Token *token) {
  struct type *result = allocate_or_error(sizeof(struct type));

  result->token = token;

  return result;
}

struct id *create_id(Token *token) {
  struct id *result = allocate_or_error(sizeof(struct id));

  result->token = token;

  return result;
}

struct stmt_list *create_stmt_list() {
  struct stmt_list *result = allocate_or_error(sizeof(struct stmt_list));

  result->head = NULL;
  result->tail = NULL;

  return result;
}

struct function *create_function(struct type *return_type, Token *name,
                                 struct stmt *body) {
  if (body->type != COMPOUND) {
    CRITICALV("create_function",
              "create_function called with wrong body! (type was %d)",
              body->type);
  }

  struct function *result = allocate_or_error(sizeof(struct function));

  result->return_type = return_type;
  result->name = create_id(name);
  result->body = body->stmt._compound.list;

  // We do not need the full stmt `body` in the new function.
  free(body);

  // record root
  root = result;

  return result;
}

struct else_stmt *create_else_stmt(struct stmt *stmt) {
  struct else_stmt *result = allocate_or_error(sizeof(struct else_stmt));

  result->body = stmt;

  return result;
}

struct stmt *create_if_stmt(struct expression *exp, struct stmt *stmt,
                            struct else_stmt *opt_else) {
  struct stmt *result = allocate_or_error(sizeof(struct stmt));

  result->type = IF;
  result->next = NULL;

  result->stmt._if.condition = exp;
  result->stmt._if.body = stmt;
  result->stmt._if.opt_else = opt_else;

  return result;
}

struct stmt *create_return_stmt(struct expression *opt_exp) {
  struct stmt *result = allocate_or_error(sizeof(struct stmt));

  result->type = RETURN;
  result->next = NULL;

  result->stmt._return.opt_exp = opt_exp;

  return result;
}

struct stmt *create_compound_stmt(struct stmt_list *list) {
  struct stmt *result = allocate_or_error(sizeof(struct stmt));

  result->type = COMPOUND;
  result->next = NULL;

  result->stmt._compound.list = list;

  return result;
}

struct stmt_list *append_stmt(struct stmt *stmt, struct stmt_list *list) {
  if (list->head == NULL) {
    list->head = stmt;
  }

  if (list->tail) {
    list->tail->next = stmt;
  }

  list->tail = stmt;

  return list;
}

struct expression *create_ternary_exp(struct expression *cond,
                                      struct expression *trueval,
                                      struct expression *falseval) {
  struct expression *result = allocate_or_error(sizeof(struct expression));

  result->type = TERNARY;
  result->exp._ternary.cond = cond;
  result->exp._ternary.true_val = trueval;
  result->exp._ternary.false_val = falseval;

  return result;
}

struct expression *create_binary_exp(struct expression *left, Token *op,
                                     struct expression *right) {
  struct expression *result = allocate_or_error(sizeof(struct expression));

  result->type = BINARY;
  result->exp._binary.left = left;
  result->exp._binary.op = op;
  result->exp._binary.right = right;

  return result;
}
struct expression *create_cast_exp(struct type *type,
                                   struct expression *child) {
  struct expression *result = allocate_or_error(sizeof(struct expression));

  result->type = CAST;
  result->exp._cast.type = type;
  result->exp._cast.child = child;

  return result;
}

struct expression *create_prefix_exp(Token *op, struct expression *child) {
  struct expression *result = allocate_or_error(sizeof(struct expression));

  result->type = PREFIX;
  result->exp._prefix.op = op;
  result->exp._prefix.child = child;

  return result;
}

struct expression *create_postfix_exp(enum postfix_exps type,
                                      struct expression *child) {
  struct expression *result = allocate_or_error(sizeof(struct expression));

  result->type = POSTFIX;
  result->exp._postfix.type = type;
  result->exp._postfix.child = child;

  return result;
}

struct expression *create_postfix_exp_index(struct expression *child,
                                            struct expression *index) {
  struct expression *result = allocate_or_error(sizeof(struct expression));

  result->type = POSTFIX;
  result->exp._postfix.type = INDEX;
  result->exp._postfix.child = child;
  result->exp._postfix.postfix.index = index;

  return result;
}

struct expression *create_postfix_exp_attr(enum postfix_exps type, Token *attr,
                                           struct expression *child) {
  struct expression *result = allocate_or_error(sizeof(struct expression));

  result->type = POSTFIX;
  result->exp._postfix.type = type;
  result->exp._postfix.child = child;
  result->exp._postfix.postfix.attribute = create_id(attr);

  return result;
}

struct expression *create_constant_exp(Token *token) {
  struct expression *result = allocate_or_error(sizeof(struct expression));

  result->type = CONSTANT;
  result->exp._constant.token = token;

  return result;
}

struct expression *create_id_exp(Token *token) {
  struct expression *result = allocate_or_error(sizeof(struct expression));

  result->type = IDENTIFIER_EXPR;
  result->exp._id.id = create_id(token);

  return result;
}

void destroy_stmt(struct stmt *stmt);
void destroy_stmt_list(struct stmt_list *stmt_list);
void destroy_expression(struct expression *exp);

void destroy_type(struct type *type) { free(type); }

void destroy_id(struct id *id) { free(id); }

void destroy_ternary_exp(struct expression *exp) {
  destroy_expression(exp->exp._ternary.cond);
  destroy_expression(exp->exp._ternary.true_val);
  destroy_expression(exp->exp._ternary.false_val);
  free(exp);
}

void destroy_binary_exp(struct expression *exp) {
  destroy_expression(exp->exp._binary.left);
  destroy_expression(exp->exp._binary.right);
  free(exp);
}

void destroy_cast_exp(struct expression *exp) {
  destroy_type(exp->exp._cast.type);
  destroy_expression(exp->exp._cast.child);
  free(exp);
}

void destroy_prefix_exp(struct expression *exp) {
  destroy_expression(exp->exp._prefix.child);
  free(exp);
}

void destroy_postfix_exp_index(struct expression *exp) {
  destroy_expression(exp->exp._postfix.postfix.index);
  destroy_expression(exp->exp._postfix.child);
  free(exp);
}

void destroy_postfix_exp_attr(struct expression *exp) {
  destroy_id(exp->exp._postfix.postfix.attribute);
  destroy_expression(exp->exp._postfix.child);
  free(exp);
}

void destroy_postfix_exp(struct expression *exp) {
  switch (exp->exp._postfix.type) {
  case INDEX:
    destroy_postfix_exp_index(exp);
    return;
  case DOT:
  case ARROW:
    destroy_postfix_exp_attr(exp);
    return;
  default: break;
  }

  destroy_expression(exp->exp._postfix.child);
  free(exp);
}

void destroy_constant_exp(struct expression *exp) { free(exp); }

void destroy_id_exp(struct expression *exp) {
  destroy_id(exp->exp._id.id);
  free(exp);
}

void destroy_expression(struct expression *exp) {
  switch (exp->type) {
  case IDENTIFIER_EXPR:
    destroy_id_exp(exp);
    break;
  case CONSTANT:
    destroy_constant_exp(exp);
    break;
  case POSTFIX:
    destroy_postfix_exp(exp);
    break;
  case PREFIX:
    destroy_prefix_exp(exp);
    break;
  case CAST:
    destroy_cast_exp(exp);
    break;
  case BINARY:
    destroy_binary_exp(exp);
    break;
  case TERNARY:
    destroy_ternary_exp(exp);
    break;
  }
}

void destroy_else_stmt(struct else_stmt *else_stmt) {
  destroy_stmt(else_stmt->body);
  free(else_stmt);
}

void destroy_if_stmt(struct if_stmt *if_stmt) {
  destroy_expression(if_stmt->condition);
  destroy_stmt(if_stmt->body);

  if (if_stmt->opt_else)
    destroy_else_stmt(if_stmt->opt_else);

  // Not dynamically allocated.
  // free(if_stmt);
}

void destroy_return_stmt(struct return_stmt *return_stmt) {
  if (return_stmt->opt_exp)
    destroy_expression(return_stmt->opt_exp);

  // Not dynamically allocated.
  // free(return_stmt);
}

void destroy_compound_stmt(struct compound_stmt *compound_stmt) {
  destroy_stmt_list(compound_stmt->list);

  // Not dynamically allocated.
  // free(compound_stmt);
}

void destroy_stmt(struct stmt *stmt) {
  switch (stmt->type) {
  case IF:
    destroy_if_stmt(&stmt->stmt._if);
    break;
  case RETURN:
    destroy_return_stmt(&stmt->stmt._return);
    break;
  case COMPOUND:
    destroy_compound_stmt(&stmt->stmt._compound);
    break;
  }

  free(stmt);
}

void destroy_stmt_list(struct stmt_list *stmt_list) {
  for (struct stmt *cur = stmt_list->head; cur != NULL;) {
    struct stmt *next = cur->next;
    destroy_stmt(cur);
    cur = next;
  }

  free(stmt_list);
}

void destroy_function(struct function *function) {
  destroy_type(function->return_type);
  destroy_id(function->name);
  destroy_stmt_list(function->body);
  free(function);
}

void destroy_tree() {
  if (root)
    destroy_function(root);
}
