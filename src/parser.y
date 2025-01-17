%{
#define _GNU_SOURCE
#include "parser.h"
#include "log.h"
#include "ast.h"
#include <string.h>
%}

%union {
  Token *tokenval;
  struct type *typeval;
  struct id *idval;
  struct exp *expval;
  struct stmt_list *listval;
  struct function *functionval;
  struct else_stmt *elseval;
  struct stmt *stmtval;
}

%token ID
%token NUM
%token K_INT "int"
%token K_RETURN "return"
%token K_IF "if"
%token K_ELSE "else"

%type<tokenval> ID
%type<typeval> type
%type<stmtval> compound_stmt
%type<functionval> function
%type<listval> stmt_list
%type<stmtval> stmt
%type<tokenval> NUM
%type<expval> exp
%type<expval> opt_exp
%type<stmtval> if_stmt
%type<elseval> opt_else_stmt
%type<elseval> else_stmt
%type<tokenval> "int"

%%
  function: type ID '(' ')' compound_stmt { $$ = create_function($1, $2, $5); }

  compound_stmt: '{' stmt_list '}' { $$ = create_compound_stmt($2); }

  stmt_list: %empty { $$ = create_stmt_list(); }
  | stmt stmt_list { $$ = append_stmt($1, $2); }

  exp: NUM { $$ = create_exp($1); }

  opt_exp: %empty { $$ = NULL; }
  | exp { $$ = $1; }

  stmt: "return" opt_exp ';' { $$ = create_return_stmt($2); }
  | if_stmt { $$ = $1; }
  | compound_stmt { $$ = $1; }

  if_stmt: "if" '(' exp ')' stmt opt_else_stmt { $$ = create_if_stmt($3, $5, $6); }

  opt_else_stmt: %empty { $$ = NULL; }
  | else_stmt { $$ = $1; }

  else_stmt: "else" stmt { $$ = create_else_stmt($2); }

  type: "int" { $$ = create_type($1); }
%%

Token *cur;
Token *next;

void init_parser(Token *list) {
#if YYDEBUG
  yydebug = 1;
#endif
  cur = NULL;
  next = list;
}

int get_keyword() {
  if (!next)
    return YYUNDEF;

  if (strcmp(next->data, "int") == 0) {
    return K_INT;
  } else if (strcmp(next->data, "return") == 0) {
    return K_RETURN;
  } else if (strcmp(next->data, "if") == 0) {
    return K_IF;
  } else if (strcmp(next->data, "else") == 0) {
    return K_ELSE;
  }

  return YYUNDEF;
}

int get_punct() {
  if (!next)
    return YYUNDEF;

  const char *comp = next->data;

  // TODO: return multi-char punctuation (eg. "->", "==", etc.)

  return (int)comp[0];
}

int yylex(void) {
  if (!next)
    return YYUNDEF;

  int ret = YYUNDEF;

  yylval.tokenval = next;
  switch (next->kind) {
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

  cur = next;
  next = next->next;

  return ret;
}

void yyerror(char const *s) {
  if (cur) {
    ERRORV("parser", "%s at token \"%s\" (line %zu:%zu)", s, cur->data,
           cur->line_number, cur->column);
  } else {
    ERROR("parser", s);
  }
}
