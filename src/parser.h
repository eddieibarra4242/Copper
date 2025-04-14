#pragma once

#include "scanner.h"
#include <stdbool.h>
#include <stdio.h>

void init_parser(Token *list);
int yyparse(void);

void free_type_alias_memory(void);

// internal
int yylex(void);
void yyerror(char const *s);
