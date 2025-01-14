#include "scanner.h"
#include "common.h"

#include <stdbool.h>
#include <string.h>

const char *keywords[] = {"int"};

bool is_alpha(char character) {
  return ('A' <= character && character <= 'Z') ||
         ('a' <= character && character <= 'z');
}

bool is_digit(char character) { return ('0' <= character && character <= '9'); }

bool is_whitespace(char character) {
  return character == ' ' || character == '\t' || character == '\r' ||
         character == '\n' || character == '\v' || character == '\f';
}

bool is_keyword(const char *value) {
  for (size_t i = 0; i < NELEMS(keywords); i++) {
    size_t length = strlen(keywords[i]);
    if (memcmp(value, keywords[i], length) == 0) {
      return true;
    }
  }

  return false;
}

size_t scan_identifier(const char *line, size_t index) {
  size_t i = index;

  while (line[i] != '\0' &&
         (is_alpha(line[i]) || is_digit(line[i]) || line[i] == '_')) {
    i++;
  }

  return i;
}

size_t scan_whitespace(const char *line, size_t index) {
  size_t i = index;

  while (line[i] != '\0' && is_whitespace(line[i])) {
    i++;
  }

  return i;
}

Token *alloc_new_token(const char *value, kind_t kind, size_t start,
                       size_t end) {
  size_t value_length = end - start;
  Token *new_token = calloc(1, sizeof(Token));

  if (!new_token) {
    return NULL;
  }

  new_token->kind = kind;
  new_token->start = start;
  new_token->end = end;
  new_token->next = NULL;

  if (!value) {
    new_token->data = NULL;
    return new_token;
  }

  char *data = malloc(value_length + 1);

  if (!data) {
    free(new_token);
    return NULL;
  }

  new_token->data = strncpy(data, value, value_length);
  new_token->data[value_length] = '\0';

  return new_token;
}

void append_linked_list(Token *new, Token **head, Token **tail) {
  if (*tail) {
    (*tail)->next = new;
  } else {
    *head = new;
  }

  *tail = new;
}

Token *scan(const char *file) {
  Token *result = NULL;
  Token *last = NULL;
  size_t i;

  for (i = 0; file[i] != '\0';) {
    kind_t kind = PUNCT;
    size_t start = i;

    if (is_alpha(file[i]) || file[i] == '_') {
      kind = IDENTIFIER;
      i = scan_identifier(file, i);
    } else if (is_whitespace(file[i])) {
      i = scan_whitespace(file, i);
      continue;
    } else {
      // consume as a character.
      i++;
    }

    const char *value_begin = &file[start];

    if (kind == IDENTIFIER && is_keyword(value_begin)) {
      kind = KEYWORD;
    }

    Token *new_token = alloc_new_token(value_begin, kind, start, i);

    if (!new_token) {
      free_list(result);
      return NULL;
    }

    append_linked_list(new_token, &result, &last);
  }

  if (!result)
    return NULL;

  Token *eof = alloc_new_token(NULL, EOF, i, i);

  if (!eof) {
    free_list(result);
    return NULL;
  }

  append_linked_list(eof, &result, &last);

  return result;
}

void free_list(Token *list) {
  if (!list)
    return;

  if (list->next)
    free_list(list->next);

  if (list->data)
    free(list->data);

  free(list);
}
