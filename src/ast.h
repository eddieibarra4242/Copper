#pragma once

#include "scanner.h"

enum stmts {
  IF,
  RETURN,
  COMPOUND,
};

struct type {
  Token *token;
};

struct id {
  Token *token;
};

struct exp {
  Token *token;
};

struct stmt;

struct else_stmt {
  struct stmt *body;
};

struct if_stmt {
  struct exp *condition;
  struct stmt *body;
  struct else_stmt *opt_else;
};

struct return_stmt {
  struct exp *opt_exp;
};

struct compound_stmt {
  struct stmt_list *list;
};

struct stmt {
  enum stmts type;
  struct stmt *next;

  union {
    struct if_stmt _if;
    struct return_stmt _return;
    struct compound_stmt _compound;
  } stmt;
};

struct stmt_list {
  struct stmt *head;
  struct stmt *tail;
};

struct function {
  struct type *return_type;
  struct id *name;
  struct stmt_list *body;
};

struct type *create_type(Token *token);
struct id *create_id(Token *token);
struct exp *create_exp(Token *token);
struct stmt_list *create_stmt_list();
struct function *create_function(struct type *return_type, Token *name,
                                 struct stmt *body);
struct else_stmt *create_else_stmt(struct stmt *stmt);
struct stmt *create_if_stmt(struct exp *exp, struct stmt *stmt,
                            struct else_stmt *opt_else);
struct stmt *create_return_stmt(struct exp *opt_exp);
struct stmt *create_compound_stmt(struct stmt_list *list);

struct stmt_list *append_stmt(struct stmt *stmt, struct stmt_list *list);

struct function *get_root();
