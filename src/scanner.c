#include "scanner.h"
#include "common.h"
#include "log.h"

#include <stdbool.h>
#include <string.h>

#define SCANNER_ERROR(expected_list)                                           \
  do {                                                                         \
    Coord pos = calc_coord(i);                                                 \
    ERRORV("scanner",                                                          \
           "Unexpected character '%c' at %zu:%zu, expected: [" expected_list   \
           "]",                                                                \
           pos.line_number, pos.column, file[i]);                              \
  } while (0)

const char *keywords[] = {
    "alignas",      "alignof",  "auto",          "bool",        "break",
    "case",         "char",     "const",         "constexpr",   "continue",
    "default",      "do",       "double",        "else",        "enum",
    "extern",       "false",    "float",         "for",         "goto",
    "if",           "inline",   "int",           "long",        "nullptr",
    "register",     "restrict", "return",        "short",       "signed",
    "sizeof",       "static",   "static_assert", "struct",      "switch",
    "thread_local", "true",     "typedef",       "typeof",      "typeof_unqual",
    "union",        "unsigned", "void",          "volatile",    "while",
    "_Atomic",      "_BitInt",  "_Complex",      "_Decimal128", "_Decimal32",
    "_Decimal64",   "_Generic", "_Imaginary",    "_Noreturn",
};

size_t seen_newlines = 0;
long last_newline = -1;

void record_newline(size_t i) {
  seen_newlines++;
  last_newline = (long)i;
}

Coord calc_coord(size_t i) {
  Coord result;

  result.line_number = seen_newlines + 1;
  result.column = (size_t)((long)i - last_newline);

  return result;
}

bool is_nondigit(char character) {
  return ('A' <= character && character <= 'Z') ||
         ('a' <= character && character <= 'z') || character == '_';
}

bool is_digit(char character) { return ('0' <= character && character <= '9'); }

bool is_hex_digit(char character) {
  return ('0' <= character && character <= '9') ||
         ('a' <= character && character <= 'f') ||
         ('A' <= character && character <= 'F');
}

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

size_t scan_whitespace(const char *file, size_t index) {
  size_t i = index;

  while (file[i] != '\0' && is_whitespace(file[i])) {
    if (file[i] == '\n') {
      record_newline(i);
    }

    i++;
  }

  return i;
}

size_t scan_reg_comment(const char *file, size_t index) {
  size_t i = index;

  while (file[i] != '\0' && file[i] != '\n') {
    i++;
  }

  return i;
}

size_t scan_inline_comment(const char *file, size_t index) {
  size_t i = index;

  while (file[i] != '\0') {
    if (file[i] == '*') {
      i++;

      if (file[i] == '/') {
        i++;
        break;
      }
    } else {
      i++;
    }
  }

  return i;
}

size_t scan_identifier(const char *file, size_t index) {
  size_t i = index;

  while (is_nondigit(file[i]) || is_digit(file[i])) {
    i++;
  }

  return i;
}

size_t scan_hex_quad(const char *file, size_t index) {
  size_t i = index;

  while ((i - index) < 4) {
    if (is_hex_digit(file[i])) {
      i++;
    } else {
      SCANNER_ERROR("0-9, a-f, A-F");
    }
  }

  return i;
}

size_t scan_universal_character(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '\\') {
    i++;
  } else {
    SCANNER_ERROR("\\");
  }

  if (file[i] == 'u') {
    i++;
    return scan_hex_quad(file, i);
  } else if (file[i] == 'U') {
    i++;
    i = scan_hex_quad(file, i);
    return scan_hex_quad(file, i);
  } else {
    SCANNER_ERROR("u, U");
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

Token *alloc_new_token(const char *value, kind_t kind, size_t start, size_t end,
                       Coord start_coord) {
  size_t value_length = end - start;
  Token *new_token = calloc(1, sizeof(Token));

  if (!new_token) {
    return NULL;
  }

  new_token->kind = kind;
  new_token->length = value_length;
  new_token->next = NULL;

  new_token->span.start = start_coord;
  new_token->span.end = calc_coord(end);

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

  // reset global state
  seen_newlines = 0;
  last_newline = -1;

  for (i = 0; file[i] != '\0';) {
    kind_t kind = PUNCT;
    size_t start = i;
    Coord start_coord = calc_coord(i);

    if (is_whitespace(file[i])) {
      i = scan_whitespace(file, i);
      continue;
    } else if (file[i] == '/') {
      i++;

      if (file[i] == '/') {
        i = scan_reg_comment(file, i);
        continue;
      } else if (file[i] == '*') {
        i++;
        i = scan_inline_comment(file, i);
        continue;
      }
    } else if (is_nondigit(file[i])) {
      kind = IDENTIFIER;
      i = scan_identifier(file, i);
    } else if (is_digit(file[i])) {
      kind = CONSTANT;
      i = scan_number(file, i);
    } else if (file[i] == '(' || file[i] == ')' || file[i] == '{' ||
               file[i] == '}' || file[i] == '?' || file[i] == ':' ||
               file[i] == '~' || file[i] == '*' || file[i] == '/' ||
               file[i] == '!' || file[i] == '%' || file[i] == ';' ||
               file[i] == '[' || file[i] == ']' || file[i] == '.') {
      i++;
    } else if (file[i] == '+') {
      i++;

      if (file[i] == '+')
        i++;
    } else if (file[i] == '-') {
      i++;

      if (file[i] == '-')
        i++;
      else if (file[i] == '>')
        i++;
    } else if (file[i] == '&') {
      i++;

      if (file[i] == '&')
        i++;
    } else if (file[i] == '|') {
      i++;

      if (file[i] == '|')
        i++;
    } else if (file[i] == '<') {
      i++;

      if (file[i] == '<')
        i++;
      else if (file[i] == '=')
        i++;
    } else if (file[i] == '>') {
      i++;

      if (file[i] == '>')
        i++;
      else if (file[i] == '=')
        i++;
    } else if (file[i] == '=') {
      i++;

      if (file[i] == '=')
        i++;
    } else {
      size_t lineno = seen_newlines + 1;
      size_t column = start - last_newline;
      ERRORV("scanner", "Unexpected character %c at %zu:%zu", file[i], lineno,
             column);
    }

    const char *value_begin = &file[start];

    if (kind == IDENTIFIER && is_keyword(value_begin)) {
      kind = KEYWORD;
    }

    Token *new_token =
        alloc_new_token(value_begin, kind, start, i, start_coord);

    if (!new_token) {
      free_list(result);
      return NULL;
    }

    append_linked_list(new_token, &result, &last);
  }

  Token *eof = alloc_new_token(NULL, EOF, i, i, calc_coord(i));

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
