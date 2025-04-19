#pragma once

#include "scanner.h"

struct symbol
{
  const char *name;
  struct symbol *next;
};

struct scope
{
  struct scope *parent;
  struct symbol *symbols;
};

void link_symbols(void);
void destroy_scope(struct scope *scope);
