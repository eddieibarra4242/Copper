#pragma once

#include "scanner.h"

struct id {
  Token *name;
};

struct declaration {
  struct id *name;

  // list
  struct declaration *next;
};

struct declaration_list {
  struct declaration *head;
  struct declaration *tail;
};

struct translation_unit {
  struct declaration_list external_declarations;
};
