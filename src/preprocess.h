#pragma once

#include "scanner.h"

struct token_list {
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

void define(Token *id, struct token_list *replace_list, Token *start,
            Token *end);
struct token_list *create_token_list(Token *start);
struct token_list *enlarge_token_list(struct token_list *list, Token *new_end);