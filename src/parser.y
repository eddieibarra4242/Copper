%{
#define _GNU_SOURCE
#include "parser.h"
#include "log.h"
#include <string.h>
%}

%glr-parser
%expect 213
%expect-rr 6

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
  /* External definitions (following A.3.4) */
  translation_unit: external_declaration
    | translation_unit external_declaration;

  external_declaration: function_definition
    | declaration;

  function_definition: attribute_specifier_sequence_opt declaration_specifiers declarator function_body;

  function_body: compound_statement;

  /* Expressions (following A.2.1) */
  primary_expression: ID
    | CONST
    | STR
    | '(' expression ')'
    | generic_selection;

  generic_selection:
    "_Generic" '(' assignment_expression ',' generic_assoc_list ')';

  generic_assoc_list: generic_assocciation
    | generic_assoc_list ',' generic_assocciation;

  generic_assocciation: type_name ':' assignment_expression
    | "default" ':' assignment_expression;

  postfix_expression: primary_expression
    | postfix_expression '[' expression ']'
    | postfix_expression '(' argument_expression_list_opt ')'
    | postfix_expression '.' ID
    | postfix_expression "->" ID
    | postfix_expression "++"
    | postfix_expression "--"
    | compound_literal;

  argument_expression_list: assignment_expression
    | argument_expression_list ',' assignment_expression;

  compound_literal: '(' storage_class_specifiers_opt type_name ')' braced_initializer;

  storage_class_specifiers: storage_class_specifier
    | storage_class_specifiers storage_class_specifier;

  unary_expression: postfix_expression
    | "++" unary_expression
    | "--" unary_expression
    | unary_op unary_expression
    | "sizeof" unary_expression
    | "sizeof" '(' type_name ')'
    | "alignof" '(' type_name ')';

  unary_op: '&'
    | '*'
    | '+'
    | '-'
    | '~'
    | '!';

  cast_expression: unary_expression
    | '(' type_name ')' cast_expression;

  multiplicative_expression: cast_expression
    | multiplicative_expression '*' cast_expression
    | multiplicative_expression '/' cast_expression
    | multiplicative_expression '%' cast_expression;

  additive_expression: multiplicative_expression
    | additive_expression '+' multiplicative_expression
    | additive_expression '-' multiplicative_expression;

  shift_expression: additive_expression
    | shift_expression "<<" additive_expression
    | shift_expression ">>" additive_expression;

  relational_expression: shift_expression
    | relational_expression '<' shift_expression
    | relational_expression '>' shift_expression
    | relational_expression "<=" shift_expression
    | relational_expression ">=" shift_expression;

  equality_expression: relational_expression
    | equality_expression "==" relational_expression
    | equality_expression "!=" relational_expression;

  and_expression: equality_expression
    | and_expression '&' equality_expression;

  xor_expression: and_expression
    | xor_expression '^' and_expression;

  or_expression: xor_expression
    | or_expression '|' xor_expression;

  logical_and_expression: or_expression
    | logical_and_expression "&&" or_expression;

  logical_or_expression: logical_and_expression
    | logical_or_expression "||" logical_and_expression;

  conditional_expression: logical_or_expression
    | logical_or_expression '?' expression ':' conditional_expression;

  assignment_expression: conditional_expression
    | unary_expression assignment_op assignment_expression;

  assignment_op: '=' | "*=" | "/=" | "%=" | "+=" | "-=" | "<<=" | ">>=" | "&=" | "^=" | "|=";

  expression: assignment_expression
    | expression ',' assignment_expression;

  constant_expression: conditional_expression;

  /* Declarations (following A.2.2) */
  declaration: declaration_specifiers ';' %dprec 2
    |  declaration_specifiers init_declarator_list ';' %dprec 1
    |  attribute_specifier_sequence declaration_specifiers init_declarator_list ';'
    |  static_assert_declaration
    |  attribute_declaration;

  declaration_specifiers: declaration_specifier attribute_specifier_sequence_opt
    | declaration_specifier declaration_specifiers;

  declaration_specifier: storage_class_specifier
    | type_specifier_qualifier
    | function_specifier;

  init_declarator_list: init_declarator
    | init_declarator_list ',' init_declarator;

  init_declarator: declarator
    | declarator '=' initializer;

  attribute_declaration: attribute_specifier_sequence ';';

  storage_class_specifier: "auto"
    | "constexpr"
    | "extern"
    | "register"
    | "static"
    | "thread_local"
    | "typedef";

  type_specifier: "void"
    | "bool"
    | "char"
    | "short"
    | "int"
    | "long"
    | "float"
    | "double"
    | "signed"
    | "unsigned"
    | "_BitInt" '(' constant_expression ')'
    | "_Complex"
    | "_Decimal32"
    | "_Decimal64"
    | "_Decimal128"
    | atomic_type_specifier
    | struct_or_union_specifier
    | enum_specifier
    | typedef_name
    | typeof_specifier;

  struct_or_union_specifier:
  struct_or_union attribute_specifier_sequence_opt identifier_opt '{' member_declaration_list '}'
    | struct_or_union attribute_specifier_sequence_opt ID;

  struct_or_union: "struct"
    | "union";

  member_declaration_list: member_declaration
    | member_declaration_list member_declaration;

  member_declaration: attribute_specifier_sequence_opt specifier_qualifier_list ';' %dprec 1
    | attribute_specifier_sequence_opt specifier_qualifier_list member_declarator_list ';' %dprec 2
    | static_assert_declaration;

  specifier_qualifier_list: type_specifier_qualifier attribute_specifier_sequence_opt
    | type_specifier_qualifier specifier_qualifier_list;

  type_specifier_qualifier: type_specifier
    | type_qualifier
    | alignment_specifier;

  member_declarator_list: member_declarator
    | member_declarator_list ',' member_declarator;

  member_declarator: declarator
    | declarator_opt ':' constant_expression;

  enum_specifier: "enum" attribute_specifier_sequence_opt identifier_opt enum_type_specifier_opt '{' enumerator_list comma_opt '}'
    | "enum" ID enum_type_specifier_opt;

  enumerator_list: enumerator
    | enumerator_list ',' enumerator;

  enumerator: enumeration_constant attribute_specifier_sequence_opt
    | enumeration_constant attribute_specifier_sequence_opt '=' constant_expression;

  enum_type_specifier: ':' specifier_qualifier_list;

  atomic_type_specifier: "_Atomic" '(' type_name ')';

  typeof_specifier: "typeof" '(' typeof_specifier_argument ')'
    | "typeof_unqual" '(' typeof_specifier_argument ')';

  typeof_specifier_argument: expression
    | type_name;

  type_qualifier: "const"
    | "restrict"
    | "volatile"
    | "_Atomic";

  function_specifier: "inline"
    | "_Noreturn";

  alignment_specifier: "alignas" '(' type_name ')'
    | "alignas" '(' constant_expression ')';

  declarator: pointer_opt direct_declarator;

  direct_declarator: ID attribute_specifier_sequence_opt
    | '(' declarator ')'
    | array_declarator attribute_specifier_sequence_opt
    | function_declarator attribute_specifier_sequence_opt;

  array_declarator: direct_declarator '[' type_qualifier_list_opt assignment_expression_opt ']'
    | direct_declarator '[' "static" type_qualifier_list_opt assignment_expression ']'
    | direct_declarator '[' type_qualifier_list "static" assignment_expression ']'
    | direct_declarator '[' type_qualifier_list_opt '*' ']';

  function_declarator: direct_declarator '(' parameter_type_list_opt ')';

  pointer: '*' attribute_specifier_sequence_opt type_qualifier_list_opt pointer_opt;

  type_qualifier_list: type_qualifier
    | type_qualifier_list type_qualifier;

  parameter_type_list: parameter_list
    | parameter_list ',' "..."
    | "...";

  parameter_list: parameter_declaration
    | parameter_list ',' parameter_declaration;

  parameter_declaration: attribute_specifier_sequence_opt declaration_specifiers declarator %dprec 2
    | attribute_specifier_sequence_opt declaration_specifiers abstract_declarator_opt %dprec 1;

  type_name: specifier_qualifier_list abstract_declarator_opt;

  abstract_declarator: pointer
    | pointer_opt direct_abstract_declarator;

  direct_abstract_declarator: '(' abstract_declarator ')'
    | array_abstract_declarator attribute_specifier_sequence_opt
    | function_abstract_declarator attribute_specifier_sequence_opt;

  array_abstract_declarator: direct_abstract_declarator_opt '[' type_qualifier_list_opt assignment_expression_opt ']'
    | direct_abstract_declarator_opt '[' "static" type_qualifier_list_opt assignment_expression ']'
    | direct_abstract_declarator_opt '[' type_qualifier_list "static" assignment_expression ']'
    | direct_abstract_declarator_opt '[' '*' ']';

  function_abstract_declarator: direct_abstract_declarator_opt '(' parameter_type_list_opt ')';

  typedef_name: ID;

  braced_initializer: '{' '}'
    | '{' initializer_list '}'
    | '{' initializer_list ',' '}';

  initializer: assignment_expression | braced_initializer;

  initializer_list_member: designation_opt initializer;
  initializer_list: initializer_list_member
    | initializer_list ',' initializer_list_member;

  designation: designator_list '=';

  designator_list: designator
    | designator_list designator;

  designator: '[' constant_expression ']'
    | '.' ID;

  static_assert_message: ',' STR;
  static_assert_message_opt: %empty | static_assert_message;
  static_assert_declaration: "static_assert" '(' constant_expression static_assert_message_opt ')' ';';

  attribute_specifier_sequence: attribute_specifier
    | attribute_specifier_sequence attribute_specifier;

  attribute_specifier: '[' '[' attribute_list ']' ']';

  attribute_list: attribute_opt
    | attribute_list ',' attribute_opt;

  attribute: attribute_token attribute_argument_clause_opt;

  attribute_token: standard_attribute
    | attribute_prefixed_token;

  standard_attribute: ID;

  attribute_prefixed_token: attribute_prefix "::" ID;

  attribute_prefix: ID;

  attribute_argument_clause: '(' balanced_token_sequence_opt ')';

  balanced_token_sequence: balanced_token
    | balanced_token_sequence balanced_token;

  balanced_token: '(' balanced_token_sequence_opt ')'
    | '[' balanced_token_sequence_opt ']'
    | '{' balanced_token_sequence_opt '}';
  // TODO: any token other than a parenthesis, a bracket, or a brace

  /* Statements (following A.3.3) */
  statement: labeled_statement
    | unlabeled_statement;

  unlabeled_statement: expression_statement
    | attribute_specifier_sequence_opt primary_block
    | attribute_specifier_sequence_opt jump_statement;

  primary_block: compound_statement
    | selection_statement
    | iteration_statement;

  secondary_block: statement;

  label: attribute_specifier_sequence_opt ID ':'
    | attribute_specifier_sequence_opt "case" constant_expression ':'
    | attribute_specifier_sequence_opt "default" ':';

  labeled_statement: label statement;

  compound_statement: '{' block_item_list_opt '}';

  block_item_list: block_item
    | block_item_list block_item;

  block_item: declaration
    | unlabeled_statement
    | label;

  expression_statement: expression_opt ';'
    | attribute_specifier_sequence expression ';';

  selection_statement: "if" '(' expression ')' secondary_block %dprec 1
    | "if" '(' expression ')' secondary_block "else" secondary_block %dprec 2
    | "switch" '(' expression ')' secondary_block;

  iteration_statement: "while" '(' expression ')' secondary_block
    | "do" secondary_block "while" '(' expression ')' ';'
    | "for" '(' expression_opt ';' expression_opt ';' expression_opt ')' secondary_block
    | "for" '(' declaration expression_opt ';' expression_opt ')' secondary_block;

  jump_statement: "goto" ID ';'
    | "continue" ';'
    | "break" ';'
    | "return" expression_opt ';';

  /* The following non-terminal definitions do not appear in spec. */
  identifier_opt: %empty | ID;
  balanced_token_sequence_opt: %empty | balanced_token_sequence;
  attribute_argument_clause_opt: %empty | attribute_argument_clause;
  attribute_opt: %empty | attribute;
  designation_opt: %empty | designation;
  direct_abstract_declarator_opt: %empty | direct_abstract_declarator;
  abstract_declarator_opt: %empty | abstract_declarator;
  parameter_type_list_opt: %empty | parameter_type_list;
  assignment_expression_opt: %empty | assignment_expression;
  type_qualifier_list_opt: %empty | type_qualifier_list;
  pointer_opt: %empty | pointer;
  declarator_opt: %empty | declarator;
  enum_type_specifier_opt: %empty | enum_type_specifier;
  argument_expression_list_opt: %empty | argument_expression_list;
  storage_class_specifiers_opt: %empty | storage_class_specifiers;
  attribute_specifier_sequence_opt: %empty | attribute_specifier_sequence;
  expression_opt: %empty | expression;
  block_item_list_opt: %empty | block_item_list;

  // For trailing comma support
  comma_opt: %empty | ',';

  // The scanner does not differentiate between ID and enumeration constants.
  enumeration_constant: ID;
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

const char *get_token_kind(void) {
  switch (cur->kind) {
  case IDENTIFIER: return "identifier";
  case KEYWORD: return "keyword";
  case PUNCT: return "symbol";
  case CONSTANT: return "constant";
  case STRING: return "string literal";
  default:
  case EOF: return "End of file";
  }
}

void yyerror(char const *s) {
  if (cur) {
    ERRORV("parser", "%s at %s \"%s\" (line %zu:%zu)", s, get_token_kind(), cur->data,
           cur->span.start.line_number, cur->span.start.column);
  } else {
    ERROR("parser", s);
  }
}
