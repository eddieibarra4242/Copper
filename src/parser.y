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
  struct expression *expval;
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

%token P_ARROW "->"
%token P_PLUS "++"
%token P_MINUS "--"

%type<tokenval> ID
%type<typeval> type
%type<stmtval> compound_stmt
%type<functionval> function
%type<listval> stmt_list
%type<stmtval> stmt
%type<tokenval> NUM
%type<expval> expression
%type<expval> postfix_expression
%type<expval> base_expression
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

  expression: postfix_expression { $$ = $1; }

  postfix_expression: base_expression { $$ = $1; }
    | postfix_expression '[' expression ']' { $$ = create_postfix_exp_index($1, $3); }
    | postfix_expression '.' ID { $$ = create_postfix_exp_attr(DOT, $3, $1); }
    | postfix_expression "->" ID { $$ = create_postfix_exp_attr(ARROW, $3, $1); }
    | postfix_expression "++" { $$ = create_postfix_exp(INCREMENT, $1); }
    | postfix_expression "--" { $$ = create_postfix_exp(DECREMENT, $1); }

  base_expression: ID { $$ = create_id_exp($1); }
    | NUM { $$ = create_constant_exp($1); }
    | '(' expression ')' { $$ = $2; }
    /* | STRING */ ;

  opt_exp: %empty { $$ = NULL; }
  | expression { $$ = $1; }

  stmt: "return" opt_exp ';' { $$ = create_return_stmt($2); }
  | if_stmt { $$ = $1; }
  | compound_stmt { $$ = $1; }

  if_stmt: "if" '(' expression ')' stmt opt_else_stmt { $$ = create_if_stmt($3, $5, $6); }

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

  if (strcmp(next->data, "->") == 0) {
    return P_ARROW;
  } else if (strcmp(next->data, "++") == 0) {
    return P_PLUS;
  } else if (strcmp(next->data, "--") == 0) {
    return P_MINUS;
  }

  if ((next->end - next->start) > 1)
    return YYUNDEF;

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
