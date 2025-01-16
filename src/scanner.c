#include "scanner.h"
#include "common.h"

#include <stdbool.h>
#include <string.h>

const char *keywords[] = {"int", "return", "if", "else"};

size_t seen_newlines = 0;
long last_newline = 0;

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

size_t scan_identifier(const char *file, size_t index) {
  size_t i = index;

  while (file[i] != '\0' &&
         (is_alpha(file[i]) || is_digit(file[i]) || file[i] == '_')) {
    i++;
  }

  return i;
}

size_t scan_whitespace(const char *file, size_t index) {
  size_t i = index;

  while (file[i] != '\0' && is_whitespace(file[i])) {
    if (file[i] == '\n') {
      seen_newlines++;
      last_newline = i;
    }

    i++;
  }

  return i;
}

size_t scan_comment(const char *file, size_t index) {
  size_t i = index;

  while (file[i] != '\0' && file[i] != '\n') {
    i++;
  }

  return i;
}

size_t scan_number(const char *file, size_t index) {
  size_t i = index;

  while (file[i] != '\0' && is_digit(file[i])) {
    i++;
  }

  if (file[i] == '.') {
    i++;
  }

  while (file[i] != '\0' && is_digit(file[i])) {
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

  // This only works if token does not contain a newline.
  new_token->line_number = seen_newlines + 1;
  new_token->column = start - last_newline;

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

  seen_newlines = 0;
  last_newline = -1;

  for (i = 0; file[i] != '\0';) {
    kind_t kind = PUNCT;
    size_t start = i;

    if (is_alpha(file[i]) || file[i] == '_') {
      kind = IDENTIFIER;
      i = scan_identifier(file, i);
    } else if (is_whitespace(file[i])) {
      i = scan_whitespace(file, i);
      continue;
    } else if (file[i] == '/') {
      i++;

      if (file[i] == '/') {
        i = scan_comment(file, i);
        continue;
      }
    } else if (is_digit(file[i])) {
      kind = NUMBER;
      i = scan_number(file, i);
    } else {
      // TODO: throw error if we see a character that we don't expect.
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
