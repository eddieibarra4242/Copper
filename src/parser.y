%{
#define _GNU_SOURCE
#include "parser.h"
#include "log.h"
#include <string.h>
%}

%union {
  Token *tokenval;
}

%token ID CONST STR

%token K_alignas "alignas"
%token K_alignof "alignof"
%token K_auto "auto"
%token K_bool "bool"
%token K_break "break"
%token K_case "case"
%token K_char "char"
%token K_const "const"
%token K_constexpr "constexpr"
%token K_continue "continue"
%token K_default "default"
%token K_do "do"
%token K_double "double"
%token K_else "else"
%token K_enum "enum"
%token K_extern "extern"
%token K_float "float"
%token K_for "for"
%token K_goto "goto"
%token K_if "if"
%token K_inline "inline"
%token K_int "int"
%token K_long "long"
%token K_register "register"
%token K_restrict "restrict"
%token K_return "return"
%token K_short "short"
%token K_signed "signed"
%token K_sizeof "sizeof"
%token K_static "static"
%token K_static_assert "static_assert"
%token K_struct "struct"
%token K_switch "switch"
%token K_thread_local "thread_local"
%token K_typedef "typedef"
%token K_typeof "typeof"
%token K_typeof_unqual "typeof_unqual"
%token K_union "union"
%token K_unsigned "unsigned"
%token K_void "void"
%token K_volatile "volatile"
%token K_while "while"
%token K__Atomic "_Atomic"
%token K__BitInt "_BitInt"
%token K__Complex "_Complex"
%token K__Decimal128 "_Decimal128"
%token K__Decimal32 "_Decimal32"
%token K__Decimal64 "_Decimal64"
%token K__Generic "_Generic"
%token K__Imaginary "_Imaginary"
%token K__Noreturn "_Noreturn"

%token P_ARROW "->"
%token P_INC "++"
%token P_DEC "--"
%token P_SHFL "<<"
%token P_SHFR ">>"
%token P_LTE "<="
%token P_GTE ">="
%token P_EE "=="
%token P_NE "!="
%token P_LAND "&&"
%token P_LOR "||"
%token P_DCOLON "::"
%token P_ELLIPSIS "..."
%token P_STARE "*="
%token P_SLASHE "/="
%token P_PERCENTE "%="
%token P_PLUSE "+="
%token P_MINUSE "-="
%token P_SHFLE "<<="
%token P_SHFRE ">>="
%token P_ANDE "&="
%token P_CARATE "^="
%token P_ORE "|="
%token P_DHASH "##"

%%
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

  if (strcmp(next->data, "alignas") == 0) return K_alignas;
  if (strcmp(next->data, "alignof") == 0) return K_alignof;
  if (strcmp(next->data, "auto") == 0) return K_auto;
  if (strcmp(next->data, "bool") == 0) return K_bool;
  if (strcmp(next->data, "break") == 0) return K_break;
  if (strcmp(next->data, "case") == 0) return K_case;
  if (strcmp(next->data, "char") == 0) return K_char;
  if (strcmp(next->data, "const") == 0) return K_const;
  if (strcmp(next->data, "constexpr") == 0) return K_constexpr;
  if (strcmp(next->data, "continue") == 0) return K_continue;
  if (strcmp(next->data, "default") == 0) return K_default;
  if (strcmp(next->data, "do") == 0) return K_do;
  if (strcmp(next->data, "double") == 0) return K_double;
  if (strcmp(next->data, "else") == 0) return K_else;
  if (strcmp(next->data, "enum") == 0) return K_enum;
  if (strcmp(next->data, "extern") == 0) return K_extern;
  if (strcmp(next->data, "float") == 0) return K_float;
  if (strcmp(next->data, "for") == 0) return K_for;
  if (strcmp(next->data, "goto") == 0) return K_goto;
  if (strcmp(next->data, "if") == 0) return K_if;
  if (strcmp(next->data, "inline") == 0) return K_inline;
  if (strcmp(next->data, "int") == 0) return K_int;
  if (strcmp(next->data, "long") == 0) return K_long;
  if (strcmp(next->data, "register") == 0) return K_register;
  if (strcmp(next->data, "restrict") == 0) return K_restrict;
  if (strcmp(next->data, "return") == 0) return K_return;
  if (strcmp(next->data, "short") == 0) return K_short;
  if (strcmp(next->data, "signed") == 0) return K_signed;
  if (strcmp(next->data, "sizeof") == 0) return K_sizeof;
  if (strcmp(next->data, "static") == 0) return K_static;
  if (strcmp(next->data, "static_assert") == 0) return K_static_assert;
  if (strcmp(next->data, "struct") == 0) return K_struct;
  if (strcmp(next->data, "switch") == 0) return K_switch;
  if (strcmp(next->data, "thread_local") == 0) return K_thread_local;
  if (strcmp(next->data, "typedef") == 0) return K_typedef;
  if (strcmp(next->data, "typeof") == 0) return K_typeof;
  if (strcmp(next->data, "typeof_unqual") == 0) return K_typeof_unqual;
  if (strcmp(next->data, "union") == 0) return K_union;
  if (strcmp(next->data, "unsigned") == 0) return K_unsigned;
  if (strcmp(next->data, "void") == 0) return K_void;
  if (strcmp(next->data, "volatile") == 0) return K_volatile;
  if (strcmp(next->data, "while") == 0) return K_while;
  if (strcmp(next->data, "_Atomic") == 0) return K__Atomic;
  if (strcmp(next->data, "_BitInt") == 0) return K__BitInt;
  if (strcmp(next->data, "_Complex") == 0) return K__Complex;
  if (strcmp(next->data, "_Decimal128") == 0) return K__Decimal128;
  if (strcmp(next->data, "_Decimal32") == 0) return K__Decimal32;
  if (strcmp(next->data, "_Decimal64") == 0) return K__Decimal64;
  if (strcmp(next->data, "_Generic") == 0) return K__Generic;
  if (strcmp(next->data, "_Imaginary") == 0) return K__Imaginary;
  if (strcmp(next->data, "_Noreturn") == 0) return K__Noreturn;

  return YYUNDEF;
}

int get_punct() {
  if (!next)
    return YYUNDEF;

  const char *comp = next->data;

  if (next->length == 1) {
    return comp[0];
  }

  if (strcmp(comp, "->") == 0) return P_ARROW;
  if (strcmp(comp, "++") == 0) return P_INC;
  if (strcmp(comp, "--") == 0) return P_DEC;
  if (strcmp(comp, "<<") == 0) return P_SHFL;
  if (strcmp(comp, ">>") == 0) return P_SHFR;
  if (strcmp(comp, "<=") == 0) return P_LTE;
  if (strcmp(comp, ">=") == 0) return P_GTE;
  if (strcmp(comp, "==") == 0) return P_EE;
  if (strcmp(comp, "!=") == 0) return P_NE;
  if (strcmp(comp, "&&") == 0) return P_LAND;
  if (strcmp(comp, "||") == 0) return P_LOR;
  if (strcmp(comp, "::") == 0) return P_DCOLON;
  if (strcmp(comp, "...") == 0) return P_ELLIPSIS;
  if (strcmp(comp, "*=") == 0) return P_STARE;
  if (strcmp(comp, "/=") == 0) return P_SLASHE;
  if (strcmp(comp, "%=") == 0) return P_PERCENTE;
  if (strcmp(comp, "+=") == 0) return P_PLUSE;
  if (strcmp(comp, "-=") == 0) return P_MINUSE;
  if (strcmp(comp, "<<=") == 0) return P_SHFLE;
  if (strcmp(comp, ">>=") == 0) return P_SHFRE;
  if (strcmp(comp, "&=") == 0) return P_ANDE;
  if (strcmp(comp, "^=") == 0) return P_CARATE;
  if (strcmp(comp, "|=") == 0) return P_ORE;
  if (strcmp(comp, "##") == 0) return P_DHASH;

  // See ISO/IEC 9899:2023 § 6.4.6 cl. 3
  if (strcmp(comp, "<:") == 0) return '[';
  if (strcmp(comp, ":>") == 0) return ']';
  if (strcmp(comp, "<%") == 0) return '{';
  if (strcmp(comp, "%>") == 0) return '}';
  if (strcmp(comp, "%:") == 0) return '#';
  if (strcmp(comp, "%:%:") == 0) return P_DHASH;

  return YYUNDEF;
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
  case CONSTANT:
    ret = CONST;
    break;
  case STRING:
    ret = STR;
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
           cur->span.start.line_number, cur->span.start.column);
  } else {
    ERROR("parser", s);
  }
}
