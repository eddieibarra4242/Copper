#pragma once

#include <stdint.h>
#include <stdlib.h>

#define IDENTIFIER 1
#define KEYWORD 2
#define PUNCT 3
#define CONSTANT 4
#define STRING 5

#ifndef EOF
// EOF should be defined as -1 anyways.
#define EOF (-1)
#endif

typedef int kind_t;

typedef struct Coord {
  size_t line_number;
  size_t column;
} Coord;

typedef struct SpanStruct {
  Coord start;
  Coord end;
} Span;

typedef struct TokenStruct {
  kind_t kind;
  size_t length;
  const char *data;
  Span span;

  struct TokenStruct *next;
} Token;

Token *scan(const char *file);
void free_list(Token *list);
