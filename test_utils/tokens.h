#pragma once

#include <stdint.h>
#include <scanner.h>

size_t get_token_list_length(Token *tokens);
void assert_coord_equal(Coord *expected, Coord *actual);
void assert_span_equal(Span *expected, Span *actual);
void assert_token_equal(Token *expected, Token *actual);
