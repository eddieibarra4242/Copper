#pragma once

#include <stdint.h>
#include <stdlib.h>

#define IDENTIFIER 1
#define KEYWORD 2
#define PUNCT 3
#define NUMBER 4

#ifndef EOF
// EOF should be defined as -1 anyways.
#define EOF (-1)
#endif

typedef int kind_t;

typedef struct TokenStruct {
  kind_t kind;
  char *data;
  size_t start;
  size_t end;

  size_t line_number;
  size_t column;

  struct TokenStruct *next;
} Token;

Token *scan(const char *file);
void free_list(Token *list);
