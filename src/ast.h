#pragma once

#include "scanner.h"
#include "stdbool.h"

enum statement_t {
  BREAK,
  COMPOUND,
  CONTINUE,
  DECL,
  EXPR,
  FOR,
  GOTO,
  IF,
  LABEL,
  RETURN,
  SWITCH,
  SWITCH_LABEL,
  WHILE,
};

enum specifier_t {
  TOKEN,
  ID_SPEC,
};

enum expression_t {
  ID_EXPR,
  CONST_EXPR,
  INDEX,
  FUNC_CALL,
  POSTFIX,
  UNARY,
  CAST,
  BINARY,
  TERNARY,
};

enum index_t {
  ARRAY,
  DOT,
  ARROW,
};

struct id {
  Token *name;
};

struct specifier {
  enum specifier_t type;

  union {
    Token *_token;
    struct id *_id;
  };

  // this is a part of a list
  struct specifier *next;
};

struct specifier_list {
  struct specifier *head;
};

struct expression;

struct initialized_declarator {
  struct id *declarator; // FIXME: replace id with declarator
  struct expression *initializer;

  // this is a part of a list
  struct initialized_declarator *next;
};

struct init_declarator_list {
  struct initialized_declarator *head;
  struct initialized_declarator *tail;
};

struct declaration {
  bool is_type_definition;

  struct specifier_list *specifiers;

  // For the next two fields, only one of them is set.
  struct id *name;
  struct init_declarator_list *init_declarator_list;

  // only for functions
  struct statement *body;

  // this is a part of a list
  struct declaration *next;
};

struct declaration_list {
  struct declaration *head;
  struct declaration *tail;
};

struct expression_list;

struct index_expr {
  enum index_t type;

  struct expression *object;
  struct expression *index;
};

struct call_expr {
  struct expression *function_ptr;
  struct expression_list *parameter_list;
};

struct unary_expr {
  Token *operator;
  struct expression *base;
};

struct cast_expr {
  struct specifier_list *type;
  struct expression *base;
};

struct binary_expr {
  Token *operator;
  struct expression *left;
  struct expression *right;
};

struct ternary_expr {
  struct expression *condition;
  struct expression *true_branch;
  struct expression *false_branch;
};

struct expression {
  enum expression_t type;

  union {
    struct id *_id;
    Token *_constant;
    struct index_expr _index;
    struct call_expr _call;
    struct unary_expr _unary;
    struct cast_expr _cast;
    struct binary_expr _binary;
    struct ternary_expr _ternary;
  };

  struct expression *next;
};

struct expression_list
{
  struct expression *head;
  struct expression *tail;
};

struct statement_list;
struct statement;

struct for_statement {
  bool is_initializer_decl;

  // For the next two fields, only one of them is set.
  struct declaration *decl;
  struct expression *preloop_expression;

  struct expression *condition;
  struct expression *step_expression;

  struct statement *body;
};

struct if_statement {
  struct expression *condition;
  struct statement *body;
  struct statement *else_body;
};

struct label {
  struct id *name;
};

struct return_statement {
  struct expression *ret_expr;
};

struct switch_label {
  struct expression *test;
};

struct switch_statement {
  struct expression *condition;
  struct statement *body;
};

struct while_statement {
  bool should_check_condition_first;
  struct expression *condition;

  struct statement *body;
};

struct statement;

struct statement_list {
  struct statement *head;
  struct statement *tail;
};

struct statement {
  enum statement_t type;

  union {
    struct statement_list _compound;
    struct declaration* _decl;
    struct expression *_expr;
    struct for_statement _for;
    struct id *_goto;
    struct if_statement _if;
    struct label _label;
    struct return_statement _return;
    struct switch_statement _switch;
    struct switch_label _switch_label;
    struct while_statement _while;
  };

  // this is a part of a list
  struct statement *next;
};

struct translation_unit {
  struct declaration_list external_declarations;
};
