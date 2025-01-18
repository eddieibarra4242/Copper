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
%token K_SIZEOF "sizeof"

%token P_ARROW "->"
%token P_PLUS "++"
%token P_MINUS "--"
%token P_SFHR ">>"
%token P_SFHL "<<"
%token P_LOR "||"
%token P_LAND "&&"
%token P_GTE ">="
%token P_LTE "<="

%type<tokenval> ID
%type<typeval> type
%type<stmtval> compound_stmt
%type<functionval> function
%type<listval> stmt_list
%type<stmtval> stmt
%type<tokenval> NUM
%type<expval> expression
%type<expval> conditional_expr
%type<expval> logical_or_expr
%type<expval> logical_and_expr
%type<expval> or_expr
%type<expval> xor_expr
%type<expval> and_expr
%type<expval> eq_expr
%type<expval> rela_expr
%type<expval> shift_expr
%type<expval> add_expr
%type<expval> mul_expr
%type<expval> cast_expr
%type<expval> prefix_expression
%type<expval> postfix_expression
%type<expval> base_expression
%type<tokenval> unary_op
%type<expval> opt_exp
%type<stmtval> if_stmt
%type<elseval> opt_else_stmt
%type<elseval> else_stmt
%type<tokenval> "int" "++" "--" "sizeof" '*' '+' '-' '~' '!' "<<"
%type<tokenval> ">>" "<=" ">=" '>' '<' "!=" "==" '&' '^' '|' "||"
%type<tokenval> "&&" '/' '%'

%%
  function: type ID '(' ')' compound_stmt { $$ = create_function($1, $2, $5); }

  compound_stmt: '{' stmt_list '}' { $$ = create_compound_stmt($2); }

  stmt_list: %empty { $$ = create_stmt_list(); }
  | stmt stmt_list { $$ = append_stmt($1, $2); }

  expression: conditional_expr { $$ = $1; }

  conditional_expr: logical_or_expr { $$ = $1; }
    | logical_or_expr '?' expression ':' conditional_expr { $$ = create_ternary_exp($1, $3, $5); }

  logical_or_expr: logical_and_expr { $$ = $1; }
    | logical_or_expr "||" logical_and_expr { $$ = create_binary_exp($1, $2, $3); }

  logical_and_expr: or_expr { $$ = $1; }
    | logical_and_expr "&&" or_expr { $$ = create_binary_exp($1, $2, $3); }

  or_expr: xor_expr { $$ = $1; }
    | or_expr '|' xor_expr { $$ = create_binary_exp($1, $2, $3); }

  xor_expr: and_expr { $$ = $1; }
    | xor_expr '^' and_expr { $$ = create_binary_exp($1, $2, $3); }

  and_expr: eq_expr { $$ = $1; }
    | and_expr '&' eq_expr { $$ = create_binary_exp($1, $2, $3); }

  eq_expr: rela_expr { $$ = $1; }
    | eq_expr "==" rela_expr { $$ = create_binary_exp($1, $2, $3); }
    | eq_expr "!=" rela_expr { $$ = create_binary_exp($1, $2, $3); }

  rela_expr: shift_expr { $$ = $1; }
    | rela_expr '>' shift_expr { $$ = create_binary_exp($1, $2, $3); }
    | rela_expr '<' shift_expr { $$ = create_binary_exp($1, $2, $3); }
    | rela_expr ">=" shift_expr { $$ = create_binary_exp($1, $2, $3); }
    | rela_expr "<=" shift_expr { $$ = create_binary_exp($1, $2, $3); }

  shift_expr: add_expr { $$ = $1; }
    | shift_expr ">>" add_expr { $$ = create_binary_exp($1, $2, $3); }
    | shift_expr "<<" add_expr { $$ = create_binary_exp($1, $2, $3); }

  add_expr: mul_expr { $$ = $1; }
    | add_expr '+' mul_expr { $$ = create_binary_exp($1, $2, $3); }
    | add_expr '-' mul_expr { $$ = create_binary_exp($1, $2, $3); }

  mul_expr: cast_expr { $$ = $1; }
    | mul_expr '*' cast_expr { $$ = create_binary_exp($1, $2, $3); }
    | mul_expr '/' cast_expr { $$ = create_binary_exp($1, $2, $3); }
    | mul_expr '%' cast_expr { $$ = create_binary_exp($1, $2, $3); }

  cast_expr: prefix_expression { $$ = $1; }
    | '(' type ')' cast_expr { $$ = create_cast_exp($2, $4); }

  prefix_expression: postfix_expression { $$ = $1; }
    | "++" prefix_expression { $$ = create_prefix_exp($1, $2); }
    | "--" prefix_expression { $$ = create_prefix_exp($1, $2); }
    | unary_op prefix_expression { $$ = create_prefix_exp($1, $2); }
    | "sizeof" prefix_expression { $$ = create_prefix_exp($1, $2); }
    /* | "sizeof" type { } */

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

  unary_op: '&' { $$ = $1; }
    | '*' { $$ = $1; }
    | '+' { $$ = $1; }
    | '-' { $$ = $1; }
    | '~' { $$ = $1; }
    | '!' { $$ = $1; }

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
  } else if (strcmp(next->data, "sizeof") == 0) {
    return K_SIZEOF;
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
  } else if (strcmp(next->data, "||") == 0) {
    return P_LOR;
  } else if (strcmp(next->data, "&&") == 0) {
    return P_LAND;
  } else if (strcmp(next->data, ">>") == 0) {
    return P_SFHR;
  } else if (strcmp(next->data, "<<") == 0) {
    return P_SFHL;
  } else if (strcmp(next->data, ">=") == 0) {
    return P_GTE;
  } else if (strcmp(next->data, "<=") == 0) {
    return P_LTE;
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
