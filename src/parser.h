#pragma once

#include "scanner.h"
#include <stdbool.h>
#include <stdio.h>

#define YYDEBUG 0

void init_parser(Token *list);
int yyparse(void);

// internal
int yylex(void);
void yyerror(char const *s);
