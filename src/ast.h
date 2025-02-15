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

struct declaration {
  bool is_type_definition;

  struct specifier_list *specifiers;
  struct id *name;

  // only for functions
  struct statement *body;

  // this is a part of a list
  struct declaration *next;
};

struct declaration_list {
  struct declaration *head;
  struct declaration *tail;
};

struct statement_list;
struct statement;

struct for_statement {
  bool is_initializer_decl;

  // initializer
  struct declaration *decl;

  // condition
  // mutator

  struct statement *body;
};

struct if_statement {
  // condition
  struct statement *body;
  struct statement *else_body;
};

struct label {
  struct id *name;
};

struct return_statement {
  // opt expression
  int FIXME;
};

struct switch_label {
  // expression
  int FIXME;
};

struct switch_statement {
  // expression
  struct statement *body;
};

struct while_statement {
  bool should_check_condition_first;
  //condition

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
    // EXPR,
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
