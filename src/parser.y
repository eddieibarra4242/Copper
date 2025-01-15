%{
#define _GNU_SOURCE
#include "parser.h"
#include "log.h"
#include <string.h>
%}

%token ID
%token NUM
%token K_INT "int"

%%
  function: type ID '(' ')' '{' '}';
  type: "int";
%%

Token *prev;
Token *current_token;

void init_parser(Token *list) {
#if YYDEBUG
  yydebug = 1;
#endif
  prev = NULL;
  current_token = list;
}

int get_keyword() {
  if (!current_token)
    return YYUNDEF;

  if (strcmp(current_token->data, "int") == 0) {
    return K_INT;
  }

  return YYUNDEF;
}

int get_punct() {
  if (!current_token)
    return YYUNDEF;

  const char *comp = current_token->data;

  // TODO: return multi-char punctuation (eg. "->", "==", etc.)

  return (int)comp[0];
}

int yylex(void) {
  if (!current_token)
    return YYUNDEF;

  int ret = YYUNDEF;

  switch (current_token->kind) {
  case IDENTIFIER:
    ret = ID;
    break;
  case KEYWORD:
    ret = get_keyword();
    break;
  case PUNCT:
    ret = get_punct();
    break;
  case NUMBER:
    ret = NUM;
    break;
  case EOF:
    ret = YYEOF;
    break;
  }

  prev = current_token;
  current_token = current_token->next;

  return ret;
}

void yyerror(char const *s) {
  if (prev) {
    ERRORV("parser", "%s at token \"%s\" (%zu:%zu)", s, prev->data,
           prev->start, prev->end);
  } else {
    ERROR("parser", s);
  }
}
