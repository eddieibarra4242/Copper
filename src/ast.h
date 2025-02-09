#pragma once

#include "scanner.h"

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

  // this is apart of a list
  struct specifier *next;
};

struct specifier_list {
  struct specifier *head;
};

struct declaration {
  struct specifier_list *specifiers;
  struct id *name;

  // this is apart of a list
  struct declaration *next;
};

struct declaration_list {
  struct declaration *head;
  struct declaration *tail;
};

struct translation_unit {
  struct declaration_list external_declarations;
};
