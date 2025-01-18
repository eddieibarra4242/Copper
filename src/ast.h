#pragma once

#include "scanner.h"

enum stmts {
  IF,
  RETURN,
  COMPOUND,
};

enum exps {
  IDENTIFIER_EXPR,
  CONSTANT,
  POSTFIX,
  PREFIX,
  CAST,
  BINARY,
  TERNARY,
};

enum postfix_exps {
  INCREMENT,
  DECREMENT,
  DOT,
  ARROW,
  FUNC_CALL,
  INDEX,
};

struct type {
  Token *token;
};

struct id {
  Token *token;
};

struct expression;

struct ternary_expression {
  struct expression *cond;
  struct expression *true_val;
  struct expression *false_val;
};

struct binary_expression {
  Token *op;
  struct expression *left;
  struct expression *right;
};

struct cast_expression {
  struct type *type;
  struct expression *child;
};

struct prefix_exp {
  Token *op;
  struct expression *child;
};

struct postfix_exp {
  enum postfix_exps type;
  struct expression *child;

  union {
    struct id *attribute;
    struct expression *index;
  } postfix;
};

struct constant_exp {
  Token *token;
};

struct id_exp {
  struct id *id;
};

struct expression {
  enum exps type;

  union {
    struct id_exp _id;
    struct constant_exp _constant;
    struct postfix_exp _postfix;
    struct prefix_exp _prefix;
    struct cast_expression _cast;
    struct binary_expression _binary;
    struct ternary_expression _ternary;
  } exp;
};

struct stmt;

struct else_stmt {
  struct stmt *body;
};

struct if_stmt {
  struct expression *condition;
  struct stmt *body;
  struct else_stmt *opt_else;
};

struct return_stmt {
  struct expression *opt_exp;
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

typedef struct function *AST;

struct type *create_type(Token *token);
struct id *create_id(Token *token);
struct stmt_list *create_stmt_list();
struct function *create_function(struct type *return_type, Token *name,
                                 struct stmt *body);
struct else_stmt *create_else_stmt(struct stmt *stmt);
struct stmt *create_if_stmt(struct expression *exp, struct stmt *stmt,
                            struct else_stmt *opt_else);
struct stmt *create_return_stmt(struct expression *opt_exp);
struct stmt *create_compound_stmt(struct stmt_list *list);

struct stmt_list *append_stmt(struct stmt *stmt, struct stmt_list *list);

struct expression *create_ternary_exp(struct expression *cond,
                                      struct expression *trueval,
                                      struct expression *falseval);

struct expression *create_binary_exp(Token *op, struct expression *right);
struct expression *set_left_binary_exp(struct expression *left,
                                       struct expression *bin_expr);

struct expression *create_cast_exp(struct type *type, struct expression *child);
struct expression *create_prefix_exp(Token *op, struct expression *child);

struct expression *create_postfix_exp(enum postfix_exps type,
                                      struct expression *child);
struct expression *create_postfix_exp_index(struct expression *child,
                                            struct expression *index);
struct expression *create_postfix_exp_attr(enum postfix_exps type, Token *attr,
                                           struct expression *child);

struct expression *create_constant_exp(Token *token);
struct expression *create_id_exp(Token *token);

AST get_tree();
void destroy_tree();
