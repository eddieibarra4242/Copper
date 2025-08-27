%{
#define _GNU_SOURCE
#include "pp_parser.h"
#include "log.h"
#include "preprocess.h"
#include <string.h>
%}

%define api.prefix {pp_}

%union {
  Token *tokenval;
  struct token_list *tokenlistval;
};

%token<tokenval> identifier keyword punctuation constant string end
%token<tokenval> K_define "define"

%type<tokenval> '#' '\n' preprocessing_token new_line
%type<tokenlistval> replacement_list pp_tokens pp_tokens_opt

%%
preprocessing_file: group_opt;

group: group_part | group group_part;

group_part: control_line | text_line | '#' new_line;

control_line: '#' "define" identifier replacement_list new_line { define($3, $4, $1, $5); }
replacement_list: pp_tokens_opt { $$ = $1; }

text_line: pp_tokens_opt new_line;

pp_tokens: preprocessing_token { $$ = create_token_list($1); }
  | pp_tokens preprocessing_token { $$ = enlarge_token_list($1, $2); }

// These rules do not appear in the spec
new_line: '\n' | end; // 'end' token is a stand-in for EOF. Since there is only one EOF, it's safe to consider EOF as a newline as well.
group_opt: %empty | group;

pp_tokens_opt: %empty { $$ = NULL; }
  | pp_tokens { $$ = $1; }

preprocessing_token: identifier
  | keyword
  | punctuation
  | constant
  | string
%%

Token *pp_cur;
Token *pp_next;

void pp_advance(void) {
  pp_cur = pp_next;
  pp_next = pp_next ? pp_next->next : NULL;

  yylval.tokenval = pp_cur;
}

void init_pp_parser(Token *first) {
#if YYDEBUG
  pp_debug = 1;
#endif
  pp_cur = NULL;
  pp_next = first;
}

int get_pp_identifier(void) {
  if (pp_cur == NULL || pp_cur->kind != IDENTIFIER) {
    return PP_UNDEF;
  }

  if (strcmp(pp_cur->data, "define") == 0) {
    return K_define;
  }

  return identifier;
}

int get_pp_punctuation(void) {
  if (pp_cur == NULL || pp_cur->kind != PUNCT) {
    return PP_UNDEF;
  }

  // See ISO/IEC 9899:2023 § 6.4.6 cl. 3
  if (strcmp(pp_cur->data, "%:") == 0) return '#';
  if (pp_cur->length != 1) return punctuation;

  switch (pp_cur->data[0]) {
    case '#': return '#';
  }

  return punctuation;
}

int pp_lex(void) {
  pp_advance();

  if (pp_cur == NULL) {
    return PP_EOF;
  }

  switch (pp_cur->kind) {
  case IDENTIFIER: return get_pp_identifier();
  case KEYWORD: return keyword;
  case PUNCT: return get_pp_punctuation();
  case CONSTANT: return constant;
  case STRING: return string;
  case NEWLINE: return '\n';
  case END: return end;
  }

  return PP_UNDEF;
}

void pp_error(const char *msg) {
  TOKEN_ERROR(preprocessor, pp_cur, msg);
}
