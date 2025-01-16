%{
#define _GNU_SOURCE
#include "parser.h"
#include "log.h"
#include <string.h>
%}

%token ID
%token NUM
%token K_INT "int"
%token K_RETURN "return"
%token K_IF "if"
%token K_ELSE "else"

%%
  function: type ID '(' ')' compound_stmt;

  compound_stmt: '{' stmt_list '}';
  stmt_list: %empty | stmt stmt_list;

  exp: NUM;

  stmt: "return" exp ';' | if_stmt | compound_stmt;

  if_stmt: "if" '(' exp ')' stmt opt_else_stmt;

  opt_else_stmt: %empty | else_stmt;
  else_stmt: "else" stmt;

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
  } else if (strcmp(current_token->data, "return") == 0) {
    return K_RETURN;
  } else if (strcmp(current_token->data, "if") == 0) {
    return K_IF;
  } else if (strcmp(current_token->data, "else") == 0) {
    return K_ELSE;
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
    ERRORV("parser", "%s at token \"%s\" (line %zu:%zu)", s, prev->data,
           prev->line_number, prev->column);
  } else {
    ERROR("parser", s);
  }
}
