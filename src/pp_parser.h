#pragma once

#include "scanner.h"

void init_pp_parser(Token *list);
int pp_parse(void);

// internal
int pp_lex(void);
void pp_error(const char *s);
