#pragma once

#include <stdint.h>
#include <stdlib.h>

typedef enum {
  IDENTIFIER,
  KEYWORD,
  PUNCT,
  CONSTANT,
  STRING,
  NEWLINE,
  END = -1,
} kind_t;

typedef struct Coord {
  size_t line_number;
  size_t column;
} Coord;

typedef struct SpanStruct {
  const char *filename;
  Coord start;
  Coord end;
} Span;

typedef struct TokenStruct {
  kind_t kind;
  size_t length;
  const char *data;
  Span span;

  struct TokenStruct *next;
} Token;

/**
 * Scans the given file contents and returns a linked list of tokens.
 * The returned list must be freed with free_list() when no longer needed.
 *
 * @param filename The name of the file being scanned
 * @param file The contents of the file to scan
 * @return A linked list of tokens, or NULL on failure
 */
Token *scan(const char *filename, const char *file);

/**
 * Frees a linked list of tokens.
 *
 * @param list The head of the linked list to free
 */
void free_list(Token *list);

/**
 * Frees a single token. Does not free any linked tokens.
 *
 * @param token The token to free
 */
void free_token(Token *token);

/**
 * Creates a copy of the given token. The returned token must be freed with
 * free_token() when no longer needed.
 *
 * @param token The token to copy
 * @return A copy of the token, or NULL on failure
 */
Token *copy_token(Token *token);

/**
 * Creates a copy of the given token list. The returned list must be freed with
 * free_list() when no longer needed.
 *
 * @param list The token list to copy
 * @return A copy of the token list, or NULL on failure
 */
Token *copy_token_list(Token *list);

/**
 * Converts a token kind to a human-readable string.
 *
 * @param kind The kind of the token
 * @return A string representation of the token kind
 */
const char *kind_to_string(kind_t kind);
