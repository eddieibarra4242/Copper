#pragma once

#include "scanner.h"

struct token_span {
  Token *start;
  Token *end;
};

/**
 * Load, scan, and preprocess a source file.
 *
 * @param filename The name of the entry file.
 * @return A linked list of tokens in pure C. (No preprocessor directives)
 */
Token *preprocess(const char *filename);

/**
 * Record a define derective.
 *
 * @param id The identifier token being defined.
 * @param replace_list The replacement token span.
 * @param start The starting token of the definition (for span).
 * @param end The ending token of the definition (for span).
 */
void define(Token *id, struct token_span *replace_list, Token *start,
            Token *end);

/**
 * Create a token span starting with the given token.
 *
 * @param start The starting token of the span.
 * @return The created token span.
 */
struct token_span *create_token_span(Token *start);

/**
 * Enlarge the token span to end with the given token.
 *
 * @param span The token span to enlarge.
 * @param new_end The new ending token of the span.
 * @return The enlarged token span.
 */
struct token_span *enlarge_token_span(struct token_span *span, Token *new_end);