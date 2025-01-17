#include "ast.h"
#include "log.h"

#include <stdlib.h>

struct function *root = NULL;

struct function *get_root() {
  return root;
}

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

struct exp *create_exp(Token *token) {
  struct exp *result = allocate_or_error(sizeof(struct exp));

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
    CRITICALV("create_function", "create_function called with wrong body! (type was %d)", body->type);
  }

  struct function *result = allocate_or_error(sizeof(struct function));

  result->return_type = return_type;
  result->name = create_id(name);
  result->body = body->stmt._compound.list;

  // record root
  root = result;

  return result;
}

struct else_stmt *create_else_stmt(struct stmt *stmt) {
  struct else_stmt *result = allocate_or_error(sizeof(struct else_stmt));

  result->body = stmt;

  return result;
}

struct stmt *create_if_stmt(struct exp *exp, struct stmt *stmt,
                            struct else_stmt *opt_else) {
  struct stmt *result = allocate_or_error(sizeof(struct stmt));

  result->type = IF;
  result->next = NULL;

  result->stmt._if.condition = exp;
  result->stmt._if.body = stmt;
  result->stmt._if.opt_else = opt_else;

  return result;
}

struct stmt *create_return_stmt(struct exp *opt_exp) {
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
