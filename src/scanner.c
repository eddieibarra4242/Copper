#include "scanner.h"
#include "common.h"
#include "log.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define SCANNER_ERROR(expected_list)                                           \
  do {                                                                         \
    Coord pos = get_current_coord();                                           \
    char found = current();                                                    \
    found = found == 0 ? '.' : found;                                          \
    found = found == '\n' ? '.' : found;                                       \
    ERRORV("scanner",                                                          \
           "Unexpected character '%c' at %zu:%zu, expected: [" expected_list   \
           "]",                                                                \
           found, pos.line_number, pos.column);                                \
  } while (0)

const char *keywords[] = {
  "alignas",       "alignof",      "auto",          "bool",
  "break",         "case",         "char",          "const",
  "constexpr",     "continue",     "default",       "do",
  "double",        "else",         "enum",          "extern",
  "float",         "for",          "goto",          "if",
  "inline",        "int",          "long",          "register",
  "restrict",      "return",       "short",         "signed",
  "sizeof",        "static",       "static_assert", "struct",
  "switch",        "thread_local", "typedef",       "typeof",
  "typeof_unqual", "union",        "unsigned",      "void",
  "volatile",      "while",        "_Atomic",       "_BitInt",
  "_Complex",      "_Decimal128",  "_Decimal32",    "_Decimal64",
  "_Generic",      "_Imaginary",   "_Noreturn",
};

const char *predefined_constants[] = {
  "false",
  "nullptr",
  "true",
};

struct ScannerState {
  size_t seen_newlines;
  long last_newline;

  size_t cur;

  const char *file;
  size_t file_length;
} state;

void initialize_scanner_state(const char *file) {
  state.seen_newlines = 0;
  state.last_newline = -1;
  state.cur = 0;

  state.file = file;
  state.file_length = strlen(file);
}

void record_newline(void) {
  state.seen_newlines++;
  state.last_newline = (long)state.cur;
}

char last(void) { return state.cur > 0 ? state.file[state.cur - 1] : EOF; }

char current(void) {
  return state.cur < state.file_length ? state.file[state.cur] : EOF;
}

char peek(void) {
  return state.cur < state.file_length - 1 ? state.file[state.cur + 1] : EOF;
}

void next(void) {
  state.cur++;

  if (current() == '\n') {
    record_newline();
  }

  if (state.file_length - state.cur >= 2 && state.file[state.cur] == '\\' &&
      state.file[state.cur + 1] == '\n') {
    state.cur++; // skip the backslash
    record_newline();
    state.cur++; // skip the newline
  }
}

Coord get_current_coord(void) {
  Coord result;

  result.line_number = state.seen_newlines + 1;
  result.column = (size_t)((long)state.cur - state.last_newline);

  return result;
}

bool is_nondigit(void) {
  char character = current();
  return ('A' <= character && character <= 'Z') ||
         ('a' <= character && character <= 'z') || character == '_';
}

#define BIN_DIGIT_SEQ "0, 1"
bool is_binary_digit(void) {
  char character = current();
  return character == '0' || character == '1';
}

bool is_char_binary_digit(char character) {
  return character == '0' || character == '1';
}

#define OCT_DIGIT_SEQ "0-7"
bool is_octal_digit(void) {
  char character = current();
  return ('0' <= character && character <= '7');
}

bool is_char_octal_digit(char character) {
  return ('0' <= character && character <= '7');
}

#define NON_ZERO_SEQ "1-9"
bool is_nonzero_digit(void) {
  char character = current();
  return ('1' <= character && character <= '9');
}

#define DEC_DIGIT_SEQ "0-9"
bool is_digit(void) {
  char character = current();
  return ('0' <= character && character <= '9');
}

bool is_char_digit(char character) {
  return ('0' <= character && character <= '9');
}

#define HEX_DIGIT_SEQ "0-9, a-f, A-F"
bool is_hex_digit(void) {
  char character = current();
  return ('0' <= character && character <= '9') ||
         ('a' <= character && character <= 'f') ||
         ('A' <= character && character <= 'F');
}

bool is_char_hex_digit(char character) {
  return ('0' <= character && character <= '9') ||
         ('a' <= character && character <= 'f') ||
         ('A' <= character && character <= 'F');
}

bool is_whitespace(void) {
  char character = current();
  return character == ' ' || character == '\t' || character == '\r' ||
         /* character == '\n' || */ character == '\v' || character == '\f';
}

bool is_in_array(const char *value, size_t value_length, const char *array[],
                 size_t len) {
  for (size_t i = 0; i < len; i++) {
    size_t constant_len = strlen(array[i]);

    if (value_length != constant_len)
      continue;

    if (strncmp(value, array[i], value_length) == 0) {
      return true;
    }
  }

  return false;
}

void scan_whitespace(void) {
  while (current() != EOF && is_whitespace()) {
    next();
  }
}

void scan_reg_comment(void) {
  while (current() != EOF && current() != '\n') {
    next();
  }
}

void scan_inline_comment(void) {
  while (current() != EOF) {
    if (current() == '*') {
      next();

      if (current() == '/') {
        next();
        break;
      }
    } else {
      next();
    }
  }
}

void scan_identifier(void) {
  while (is_nondigit() || is_digit()) {
    next();
  }
}

void scan_hex_quad(void) {
  size_t start_index = state.cur;
  while ((state.cur - start_index) < 4) {
    if (is_hex_digit()) {
      next();
    } else {
      SCANNER_ERROR(HEX_DIGIT_SEQ);
    }
  }
}

void scan_universal_character(void) {
  if (current() == '\\') {
    next();
  } else {
    SCANNER_ERROR("\\");
  }

  char size = current();
  next();

  if (size == 'U') {
    scan_hex_quad();
  } else if (size != 'u') {
    SCANNER_ERROR("u, U");
  }

  scan_hex_quad();
}

void scan_sign(void) {
  if (current() == '+' || current() == '-') {
    next();
  }
}

void scan_floating_suffix(void) {
  static const char *suffixes[] = {"f",  "l",  "F",  "L",  "df",
                                   "dd", "dl", "DF", "DD", "DL"};

  size_t found_length = 0;

  for (size_t i = 0; i < NELEMS(suffixes); i++) {
    size_t length = strlen(suffixes[i]);
    if (memcmp(&state.file[state.cur], suffixes[i], length) == 0) {
      found_length = length;
      break;
    }
  }

  for (size_t i = 0; i < found_length; i++) {
    next();
  }
}

void scan_digit_seq(void) {
  if (is_digit()) {
    next();
  } else {
    SCANNER_ERROR(DEC_DIGIT_SEQ);
  }

  while (is_digit() || current() == '\'') {
    next();
  }

  if (!is_char_digit(last())) {
    state.cur--;
    SCANNER_ERROR(DEC_DIGIT_SEQ);
  }
}

void scan_hex_digit_seq(void) {
  if (is_hex_digit()) {
    next();
  } else {
    SCANNER_ERROR(HEX_DIGIT_SEQ);
  }

  while (is_hex_digit() || current() == '\'') {
    next();
  }

  if (!is_char_hex_digit(last())) {
    state.cur--;
    SCANNER_ERROR(HEX_DIGIT_SEQ);
  }
}

void scan_binary_exp(void) {
  if (current() == 'p' || current() == 'P') {
    next();
  }

  scan_sign();
  scan_digit_seq();
}

void scan_exp(void) {
  if (current() == 'e' || current() == 'E') {
    next();
  } else {
    SCANNER_ERROR("e, E");
  }

  scan_sign();
  scan_digit_seq();
}

void scan_fractional_const(void) { scan_digit_seq(); }

void scan_period(void) {
  if (current() == '.') {
    next();
  } else {
    SCANNER_ERROR(".");
  }

  if (is_digit()) {
    scan_fractional_const();
  }
}

void scan_hex_fractional_const(void) {
  if (current() == '.') {
    next();
  } else {
    SCANNER_ERROR(".");
  }

  scan_hex_digit_seq();
}

void scan_int_prefix_rest(void) {
  if (current() == 'w') {
    next();

    if (current() == 'b') {
      next();
    } else {
      SCANNER_ERROR("b");
    }
  } else if (current() == 'W') {
    next();

    if (current() == 'B') {
      next();
    } else {
      SCANNER_ERROR("B");
    }
  } else if (current() == 'l') {
    next();

    if (current() == 'l') {
      next();
    } else if (current() == 'L') {
      SCANNER_ERROR("l");
    }
  } else if (current() == 'L') {
    next();

    if (current() == 'L') {
      next();
    } else if (current() == 'l') {
      SCANNER_ERROR("L");
    }
  }
}

void scan_int_prefix_ufirst(void) {
  if (current() == 'u' || current() == 'U') {
    next();
  }

  scan_int_prefix_rest();
}

void scan_int_prefix_ulast(void) {
  scan_int_prefix_rest();

  if (current() == 'u' || current() == 'U') {
    next();
  }
}

void scan_int_prefix_opt(void) {
  if (current() == 'u' || current() == 'U') {
    scan_int_prefix_ufirst();
  } else if (current() == 'w' || current() == 'W' || current() == 'l' ||
             current() == 'L') {
    scan_int_prefix_ulast();
  }
}

void scan_binary_number(void) {
  if (current() == '0') {
    next();
  } else {
    SCANNER_ERROR("0");
  }

  if (current() == 'b' || current() == 'B') {
    next();
  } else {
    SCANNER_ERROR("b, B");
  }

  if (is_binary_digit()) {
    next();
  } else {
    SCANNER_ERROR(BIN_DIGIT_SEQ);
  }

  while (is_hex_digit() || current() == '\'') {
    if (!is_binary_digit() && current() != '\'') {
      SCANNER_ERROR(BIN_DIGIT_SEQ);
    }

    next();
  }

  if (!is_char_binary_digit(last())) {
    state.cur--;
    SCANNER_ERROR(BIN_DIGIT_SEQ);
  }

  scan_int_prefix_opt();
}

void scan_octal_number(void) {
  if (current() == '0') {
    next();
  } else {
    SCANNER_ERROR("0");
  }

  if (current() == '.') {
    scan_period();

    if (current() == 'e' || current() == 'E') {
      scan_exp();
    }

    scan_floating_suffix();
    return;
  }

  while (is_hex_digit() || current() == '\'') {
    if (!is_octal_digit() && current() != '\'') {
      SCANNER_ERROR(OCT_DIGIT_SEQ);
    }

    next();
  }

  if (!is_char_octal_digit(last())) {
    state.cur--;
    SCANNER_ERROR(OCT_DIGIT_SEQ);
  }

  scan_int_prefix_opt();
}

void scan_decimal_number(void) {
  if (is_nonzero_digit()) {
    scan_digit_seq();

    if (current() == '.') {
      scan_period();

      if (current() == 'e' || current() == 'E') {
        scan_exp();
      }

      scan_floating_suffix();
    }
  } else {
    SCANNER_ERROR(NON_ZERO_SEQ);
  }

  scan_int_prefix_opt();
}

void scan_hex_number(void) {
  if (current() == '0') {
    next();
  } else {
    SCANNER_ERROR("0");
  }

  if (current() == 'x' || current() == 'X') {
    next();
  } else {
    SCANNER_ERROR("x, X");
  }

  if (!is_hex_digit() && current() != '.') {
    SCANNER_ERROR(HEX_DIGIT_SEQ);
  }

  if (is_hex_digit()) {
    scan_hex_digit_seq();
  }

  if (current() == '.') {
    scan_hex_fractional_const();
    scan_binary_exp();
    scan_floating_suffix();
  }

  scan_int_prefix_opt();
}

void scan_number(void) {
  if (current() == '0') {
    if (peek() == 'x' || peek() == 'X') {
      scan_hex_number();
    } else if (peek() == 'b' || peek() == 'B') {
      scan_binary_number();
    } else {
      scan_octal_number();
    }
  } else if (is_digit()) {
    scan_decimal_number();
  } else {
    SCANNER_ERROR(DEC_DIGIT_SEQ);
  }
}

void scan_simple_esc(void) {
  if (current() == '\'' || current() == '"' || current() == '?' ||
      current() == '\\' || current() == 'a' || current() == 'b' ||
      current() == 'f' || current() == 'n' || current() == 'r' ||
      current() == 't' || current() == 'v') {
    next();
  } else {
    SCANNER_ERROR("', \", ?, \\, a, b, f, n, r, t, v");
  }
}

void scan_octal_esc(void) {
  if (is_octal_digit()) {
    next();
  } else {
    SCANNER_ERROR(OCT_DIGIT_SEQ);
  }

  if (is_octal_digit()) {
    next();
  }

  if (is_octal_digit()) {
    next();
  }
}

void scan_hexadecimal_esc(void) {
  if (current() == '\\') {
    next();
  } else {
    SCANNER_ERROR("\\");
  }

  if (current() == 'x') {
    next();
  } else {
    SCANNER_ERROR("x");
  }

  if (is_hex_digit()) {
    next();
  } else {
    SCANNER_ERROR(HEX_DIGIT_SEQ);
  }

  while (is_hex_digit()) {
    next();
  }
}

void scan_char(void) {
  if (memcmp(&state.file[state.cur], "\\u", 2) == 0) {
    scan_universal_character();
  } else if (memcmp(&state.file[state.cur], "\\x", 2) == 0) {
    scan_hexadecimal_esc();
  } else if (current() == '\\') {
    next();

    if (is_octal_digit()) {
      scan_octal_esc();
    } else {
      scan_simple_esc();
    }
  } else {
    next();
  }
}

void scan_c_char_seq(void) {
  if (current() == '\'') {
    next();
  } else {
    SCANNER_ERROR("\'");
  }

  if (current() == '\n') {
    SCANNER_ERROR("a char");
  }

  scan_char();

  if (current() == '\'') {
    next();
  } else {
    SCANNER_ERROR("\'");
  }
}

void scan_s_char_seq(void) {
  if (current() == '"') {
    next();
  } else {
    SCANNER_ERROR("\"");
  }

  while (current() != '"') {
    if (current() == '\n') {
      SCANNER_ERROR("\"");
    }

    scan_char();
  }

  if (current() == '"') {
    next();
  } else {
    SCANNER_ERROR("\"");
  }
}

Token *alloc_new_token(const char *filename, const char *value, kind_t kind,
                       size_t start, Coord start_coord) {
  size_t end = state.cur;
  size_t value_length = end - start;
  Token *new_token = calloc(1, sizeof(Token));

  if (!new_token) {
    return NULL;
  }

  new_token->kind = kind;
  new_token->length = value_length;
  new_token->next = NULL;

  new_token->span.filename = filename;
  new_token->span.start = start_coord;
  new_token->span.end = get_current_coord();

  if (!value || kind == NEWLINE) {
    new_token->data = NULL;
    return new_token;
  }

  char *data = malloc(value_length + 1);

  if (!data) {
    free(new_token);
    return NULL;
  }

  size_t d_index = 0;
  for (size_t i = 0; i < value_length; i++) {
    if (value[i] == '\\' && value[i + 1] == '\n') {
      i++;
      continue;
    }

    data[d_index++] = value[i];
  }

  data[d_index] = '\0';
  new_token->data = data;
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

Token *scan(const char *filename, const char *file) {
  Token *result = NULL;
  Token *last = NULL;

  initialize_scanner_state(file);

  while (current() != EOF) {
    kind_t kind = PUNCT;
    size_t start = state.cur;
    Coord start_coord = get_current_coord();

    if (is_whitespace()) {
      scan_whitespace();
      continue;
    } else if (current() == '\n') {
      next();
      kind = NEWLINE;
    } else if (current() == '/') {
      next();

      if (current() == '/') {
        scan_reg_comment();
        continue;
      } else if (current() == '*') {
        next();
        scan_inline_comment();
        continue;
      } else if (current() == '=') {
        next();
      }
    } else if (current() == 'u') {
      kind = IDENTIFIER;
      next();

      if (current() == '8') {
        kind = CONSTANT;
        next();

        if (current() == '\'') {
          scan_c_char_seq();
        } else if (current() == '\"') {
          kind = STRING;
          scan_s_char_seq();
        }
      } else if (current() == '\'') {
        kind = CONSTANT;
        scan_c_char_seq();
      } else if (current() == '\"') {
        kind = STRING;
        scan_s_char_seq();
      } else {
        scan_identifier();
      }
    } else if (current() == 'U' || current() == 'L') {
      kind = IDENTIFIER;
      next();

      if (current() == '\'') {
        kind = CONSTANT;
        scan_c_char_seq();
      } else if (current() == '\"') {
        kind = STRING;
        scan_s_char_seq();
      } else {
        scan_identifier();
      }
    } else if (is_nondigit()) {
      kind = IDENTIFIER;
      scan_identifier();
    } else if (is_digit()) {
      kind = CONSTANT;
      scan_number();
    } else if (current() == '.') {
      next();

      if (is_digit()) {
        kind = CONSTANT;
        scan_fractional_const();
        scan_floating_suffix();
      } else if (current() == '.') {
        next();

        if (current() == '.') {
          next();
        } else {
          SCANNER_ERROR(".");
        }
      }
    } else if (current() == '\'') {
      kind = CONSTANT;
      scan_c_char_seq();
    } else if (current() == '\"') {
      kind = STRING;
      scan_s_char_seq();
    } else if (current() == '[' || current() == ']' || current() == '(' ||
               current() == ')' || current() == '{' || current() == '}' ||
               current() == '~' || current() == '?' || current() == ';' ||
               current() == ',') {
      next();
    } else if (current() == '-') {
      next();

      if (current() == '>' || current() == '-' || current() == '=') {
        next();
      }
    } else if (current() == '+') {
      next();

      if (current() == '+' || current() == '=') {
        next();
      }
    } else if (current() == '<') {
      next();

      if (current() == '<') {
        next();

        if (current() == '=') {
          next();
        }
      } else if (current() == '=' || current() == ':' || current() == '%') {
        next();
      }
    } else if (current() == '>') {
      next();

      if (current() == '>') {
        next();

        if (current() == '=') {
          next();
        }
      } else if (current() == '=') {
        next();
      }
    } else if (current() == '=') {
      next();

      if (current() == '=') {
        next();
      }
    } else if (current() == '!') {
      next();

      if (current() == '=') {
        next();
      }
    } else if (current() == '&') {
      next();

      if (current() == '=' || current() == '&') {
        next();
      }
    } else if (current() == '|') {
      next();

      if (current() == '=' || current() == '|') {
        next();
      }
    } else if (current() == ':') {
      next();

      if (current() == ':' || current() == '>') {
        next();
      }
    } else if (current() == '*') {
      next();

      if (current() == '=') {
        next();
      }
    } else if (current() == '%') {
      next();

      if (current() == '=' || current() == '>') {
        next();
      } else if (current() == ':') {
        next();

        if (current() == '%') {
          next();

          if (current() == ':') {
            next();
          } else {
            SCANNER_ERROR(":");
          }
        }
      }
    } else if (current() == '^') {
      next();

      if (current() == '=') {
        next();
      }
    } else if (current() == '#') {
      next();

      if (current() == '#') {
        next();
      }
    } else {
      Coord coord = get_current_coord();
      ERRORV("scanner", "Unexpected character %c at %zu:%zu", current(),
             coord.line_number, coord.column);
    }

    const char *value_begin = &file[start];
    size_t value_length = state.cur - start;

    if (kind == IDENTIFIER &&
        is_in_array(value_begin, value_length, keywords, NELEMS(keywords))) {
      kind = KEYWORD;
    } else if (kind == IDENTIFIER &&
               is_in_array(value_begin, value_length, predefined_constants,
                           NELEMS(predefined_constants))) {
      kind = CONSTANT;
    }

    Token *new_token =
      alloc_new_token(filename, value_begin, kind, start, start_coord);

    if (!new_token) {
      free_list(result);
      return NULL;
    }

    append_linked_list(new_token, &result, &last);
  }

  Token *eof =
    alloc_new_token(filename, NULL, END, state.cur, get_current_coord());

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

  free_token(list);
}

void free_token(Token *token) {
  if (!token)
    return;

  if (token->data)
    free((void *)token->data);

  free(token);
}

Token *copy_token(Token *token) {
  if (!token)
    return NULL;

  Token *new_token = calloc(1, sizeof(Token));

  if (!new_token)
    return NULL;

  new_token->kind = token->kind;
  new_token->length = token->length;
  new_token->span = token->span;

  if (token->data) {
    char *data = malloc(token->length + 1);

    if (!data) {
      free(new_token);
      return NULL;
    }

    memcpy(data, token->data, token->length);
    data[token->length] = '\0';
    new_token->data = data;
  }

  return new_token;
}

Token *copy_token_list(Token *list) {
  if (!list)
    return NULL;

  Token *result = NULL;
  Token *last = NULL;

  for (Token *cur = list; cur != NULL; cur = cur->next) {
    Token *new_token = copy_token(cur);

    if (!new_token) {
      free_list(result);
      return NULL;
    }

    append_linked_list(new_token, &result, &last);
  }

  return result;
}

const char *kind_to_string(kind_t kind) {
  switch (kind) {
  case IDENTIFIER:
    return "identifier";
  case KEYWORD:
    return "keyword";
  case PUNCT:
    return "symbol";
  case CONSTANT:
    return "constant";
  case STRING:
    return "string literal";
  case NEWLINE:
    return "line break";
  default:
  case END:
    return "End of file";
  }
}
