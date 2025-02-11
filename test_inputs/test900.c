// This is the our scanner without any preprocessing stuff.

typedef unsigned long size_t;
typedef char bool;

typedef int kind_t;

typedef struct Coord {
  size_t line_number;
  size_t column;
} Coord;

typedef struct SpanStruct {
  Coord start;
  Coord end;
} Span;

typedef struct TokenStruct {
  kind_t kind;
  size_t length;
  char *data;
  Span span;

  struct TokenStruct *next;
} Token;

Token *scan(const char *file);
void free_list(Token *list);

const void *NULL = 0;

const int IDENTIFIER = 1;
const int KEYWORD = 2;
const int PUNCT = 3;
const int CONSTANT = 4;
const int STRING = 5;
const int EOF = (-1);

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

bool is_octal_digit(char character) {
  return ('0' <= character && character <= '7');
}

const char *NON_ZERO_SEQ = "1-9";
bool is_nonzero_digit(char character) {
  return ('1' <= character && character <= '9');
}

const char *DEC_DIGIT_SEQ = "0-9";
bool is_digit(char character) { return ('0' <= character && character <= '9'); }

const char *HEX_DIGIT_SEQ = "0-9, a-f, A-F";
bool is_hex_digit(char character) {
  return ('0' <= character && character <= '9') ||
         ('a' <= character && character <= 'f') ||
         ('A' <= character && character <= 'F');
}

bool is_whitespace(char character) {
  return character == ' ' || character == '\t' || character == '\r' ||
         character == '\n' || character == '\v' || character == '\f';
}

bool is_in_array(const char *value, size_t value_length, const char *array[],
                 size_t len) {
  for (size_t i = 0; i < len; i++) {
    size_t constant_len = strlen(array[i]);

    if (value_length != constant_len)
      continue;

    if (strncmp(value, array[i], value_length) == 0) {
      return 1;
    }
  }

  return 0;
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
      SCANNER_ERROR(HEX_DIGIT_SEQ);
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

size_t scan_sign(const char *file, size_t index) {
  if (file[index] == '+') {
    return index + 1;
  } else if (file[index] == '-') {
    return index + 1;
  }

  return index;
}

size_t scan_floating_suffix(const char *file, size_t index) {
  static const char *suffixes[] = {
      "f", "l", "F", "L", "df", "dd", "dl", "DF", "DD", "DL",
  };

  for (size_t i = 0; i < NELEMS(suffixes); i++) {
    size_t length = strlen(suffixes[i]);
    if (memcmp(&file[index], suffixes[i], length) == 0) {
      return index + length;
    }
  }

  return index;
}

size_t scan_digit_seq(const char *file, size_t index) {
  size_t i = index;

  if (is_digit(file[i])) {
    i++;
  } else {
    SCANNER_ERROR(DEC_DIGIT_SEQ);
  }

  while (is_digit(file[i]) || file[i] == '\'') {
    i++;
  }

  i--;
  if (is_digit(file[i])) {
    i++;
  } else {
    SCANNER_ERROR(DEC_DIGIT_SEQ);
  }

  return i;
}

size_t scan_hex_digit_seq(const char *file, size_t index) {
  size_t i = index;

  if (is_hex_digit(file[i])) {
    i++;
  } else {
    SCANNER_ERROR(HEX_DIGIT_SEQ);
  }

  while (is_hex_digit(file[i]) || file[i] == '\'') {
    i++;
  }

  i--;
  if (is_hex_digit(file[i])) {
    i++;
  } else {
    SCANNER_ERROR(HEX_DIGIT_SEQ);
  }

  return i;
}

size_t scan_binary_exp(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == 'p' || file[i] == 'P') {
    i++;
  }

  i = scan_sign(file, i);

  return scan_digit_seq(file, i);
}

size_t scan_exp(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == 'e' || file[i] == 'E') {
    i++;
  } else {
    SCANNER_ERROR("e, E");
  }

  i = scan_sign(file, i);

  return scan_digit_seq(file, i);
}

size_t scan_fractional_const(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '.') {
    i++;
  } else {
    SCANNER_ERROR(".");
  }

  return scan_digit_seq(file, i);
}

size_t scan_period(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '.') {
    i++;
  } else {
    SCANNER_ERROR(".");
  }

  if (is_digit(file[i])) {
    return scan_fractional_const(file, index);
  }

  return i;
}

size_t scan_hex_fractional_const(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '.') {
    i++;
  } else {
    SCANNER_ERROR(".");
  }

  return scan_hex_digit_seq(file, i);
}

size_t scan_hex_fractional_const_opt(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '.') {
    i++;
  } else {
    SCANNER_ERROR(".");
  }

  if (is_hex_digit(file[i])) {
    return scan_fractional_const(file, index);
  }

  return i;
}

size_t scan_int_prefix_rest(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == 'w') {
    i++;

    if (file[i] == 'b') {
      i++;
    } else {
      SCANNER_ERROR("b");
    }
  } else if (file[i] == 'W') {
    i++;

    if (file[i] == 'B') {
      i++;
    } else {
      SCANNER_ERROR("B");
    }
  } else if (file[i] == 'l') {
    i++;

    if (file[i] == 'l') {
      i++;
    } else if (file[i] == 'L') {
      SCANNER_ERROR("l");
    }
  } else if (file[i] == 'L') {
    i++;

    if (file[i] == 'L') {
      i++;
    } else if (file[i] == 'l') {
      SCANNER_ERROR("L");
    }
  }

  return i;
}

size_t scan_int_prefix_ufirst(const char *file, size_t index) {
  size_t i = index;

  if (file[index] == 'u' || file[index] == 'U') {
    i++;
  }

  return scan_int_prefix_rest(file, i);
}

size_t scan_int_prefix_ulast(const char *file, size_t index) {
  size_t i = scan_int_prefix_rest(file, index);

  if (file[i] == 'u' || file[i] == 'U') {
    i++;
  }

  return i;
}

size_t scan_int_prefix_opt(const char *file, size_t index) {
  if (file[index] == 'u' || file[index] == 'U') {
    return scan_int_prefix_ufirst(file, index);
  } else if (file[index] == 'w' || file[index] == 'W' || file[index] == 'l' ||
             file[index] == 'L') {
    return scan_int_prefix_ulast(file, index);
  }

  return index;
}

size_t scan_binary_number(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '0') {
    i++;
  } else {
    SCANNER_ERROR("0");
  }

  if (file[i] == 'b' || file[i] == 'B') {
    i++;
  } else {
    SCANNER_ERROR("b, B");
  }

  if (file[i] == '0' || file[i] == '1') {
    i++;
  } else {
    SCANNER_ERROR("0, 1");
  }

  while (is_hex_digit(file[i]) || file[i] == '\'') {
    if (file[i] != '0' && file[i] != '1' && file[i] != '\'') {
      SCANNER_ERROR("0, 1");
    }

    i++;
  }

  i--;
  if (file[i] == '0' || file[i] == '1') {
    i++;
  } else {
    SCANNER_ERROR("0, 1");
  }

  return scan_int_prefix_opt(file, i);
}

size_t scan_octal_number(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '0') {
    i++;
  } else {
    SCANNER_ERROR("0");
  }

  if (file[i] == '.') {
    i = scan_period(file, i);

    if (file[i] == 'e' || file[i] == 'E') {
      i = scan_exp(file, i);
    }

    return scan_floating_suffix(file, i);
  }

  while (is_hex_digit(file[i]) || file[i] == '\'') {
    if (!is_octal_digit(file[i]) && file[i] != '\'') {
      SCANNER_ERROR("0-7");
    }

    i++;
  }

  i--;
  if (is_octal_digit(file[i])) {
    i++;
  } else {
    SCANNER_ERROR("0-7");
  }

  return scan_int_prefix_opt(file, i);
}

size_t scan_decimal_number(const char *file, size_t index) {
  size_t i = index;

  if (is_nonzero_digit(file[i])) {
    i = scan_digit_seq(file, i);

    if (file[i] == '.') {
      i = scan_period(file, i);

      if (file[i] == 'e' || file[i] == 'E') {
        i = scan_exp(file, i);
      }

      i = scan_floating_suffix(file, i);
    }
  } else {
    SCANNER_ERROR(NON_ZERO_SEQ);
  }

  return scan_int_prefix_opt(file, i);
}

size_t scan_hex_number(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '0') {
    i++;
  } else {
    SCANNER_ERROR("0");
  }

  if (file[i] == 'x' || file[i] == 'X') {
    i++;
  } else {
    SCANNER_ERROR("x, X");
  }

  if (is_hex_digit(file[i])) {
    i = scan_hex_digit_seq(file, i);

    if (file[i] == '.') {
      i = scan_hex_fractional_const_opt(file, i);
      i = scan_binary_exp(file, i);
      i = scan_floating_suffix(file, i);
    }
  } else if (file[i] == '.') {
    i = scan_hex_fractional_const(file, i);
    i = scan_binary_exp(file, i);
    i = scan_floating_suffix(file, i);
  } else {
    SCANNER_ERROR(HEX_DIGIT_SEQ);
  }

  return scan_int_prefix_opt(file, i);
}

size_t scan_number(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '0') {
    if (file[i + 1] == 'x' || file[i + 1] == 'X') {
      return scan_hex_number(file, i);
    } else if (file[i + 1] == 'b' || file[i + 1] == 'B') {
      return scan_binary_number(file, i);
    }

    return scan_octal_number(file, i);
  } else if (is_digit(file[i])) {
    return scan_decimal_number(file, i);
  } else {
    SCANNER_ERROR(DEC_DIGIT_SEQ);
  }

  return i;
}

size_t scan_simple_esc(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '\'' || file[i] == '"' || file[i] == '?' || file[i] == '\\' ||
      file[i] == 'a' || file[i] == 'b' || file[i] == 'f' || file[i] == 'n' ||
      file[i] == 'r' || file[i] == 't' || file[i] == 'v') {
    i++;
  } else {
    SCANNER_ERROR("', \", ?, \\, a, b, f, n, r, t, v");
  }

  return i;
}

size_t scan_octal_esc(const char *file, size_t index) {
  size_t i = index;

  if (is_octal_digit(file[i])) {
    i++;
  } else {
    SCANNER_ERROR("0-7");
  }

  if (is_octal_digit(file[i])) {
    i++;
  }

  if (is_octal_digit(file[i])) {
    i++;
  }

  return i;
}

size_t scan_hexadecimal_esc(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '\\') {
    i++;
  } else {
    SCANNER_ERROR("\\");
  }

  if (file[i] == 'x') {
    i++;
  } else {
    SCANNER_ERROR("x");
  }

  if (is_hex_digit(file[i])) {
    i++;
  } else {
    SCANNER_ERROR(HEX_DIGIT_SEQ);
  }

  while (is_hex_digit(file[i])) {
    i++;
  }

  return i;
}

size_t scan_char(const char *file, size_t index) {
  size_t i = index;

  if (memcmp(&file[i], "\\u", 2) == 0) {
    i = scan_universal_character(file, index);
  } else if (memcmp(&file[i], "\\x", 2) == 0) {
    i = scan_hexadecimal_esc(file, index);
  } else if (file[i] == '\\') {
    i++;

    if (is_octal_digit(file[i])) {
      i = scan_octal_esc(file, i);
    } else {
      i = scan_simple_esc(file, i);
    }
  } else {
    i++;
  }

  return i;
}

size_t scan_c_char_seq(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '\'') {
    i++;
  } else {
    SCANNER_ERROR("\'");
  }

  if (file[i] == '\n') {
    SCANNER_ERROR("a char");
  }

  i = scan_char(file, i);

  if (file[i] == '\'') {
    i++;
  } else {
    SCANNER_ERROR("\'");
  }

  return i;
}

size_t scan_s_char_seq(const char *file, size_t index) {
  size_t i = index;

  if (file[i] == '"') {
    i++;
  } else {
    SCANNER_ERROR("\"");
  }

  while (file[i] != '\"') {
    if (file[i] == '\n') {
      SCANNER_ERROR("\"");
    }

    i = scan_char(file, i);
  }

  if (file[i] == '"') {
    i++;
  } else {
    SCANNER_ERROR("\"");
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
      } else if (file[i] == '=') {
        i++;
      }
    } else if (file[i] == 'u') {
      kind = IDENTIFIER;
      i++;

      if (file[i] == '8') {
        kind = CONSTANT;
        i++;

        if (file[i] == '\'') {
          i = scan_c_char_seq(file, i);
        } else if (file[i] == '\"') {
          kind = STRING;
          i = scan_s_char_seq(file, i);
        }
      } else if (file[i] == '\'') {
        kind = CONSTANT;
        i = scan_c_char_seq(file, i);
      } else if (file[i] == '\"') {
        kind = STRING;
        i = scan_s_char_seq(file, i);
      } else {
        i = scan_identifier(file, i);
      }
    } else if (file[i] == 'U' || file[i] == 'L') {
      kind = IDENTIFIER;
      i++;

      if (file[i] == '\'') {
        kind = CONSTANT;
        i = scan_c_char_seq(file, i);
      } else if (file[i] == '\"') {
        kind = STRING;
        i = scan_s_char_seq(file, i);
      } else {
        i = scan_identifier(file, i);
      }
    } else if (is_nondigit(file[i])) {
      kind = IDENTIFIER;
      i = scan_identifier(file, i);
    } else if (is_digit(file[i])) {
      kind = CONSTANT;
      i = scan_number(file, i);
    } else if (file[i] == '.') {
      i++;

      if (is_digit(file[i])) {
        kind = CONSTANT;
        i = scan_fractional_const(file, i - 1);
        i = scan_floating_suffix(file, i);
      } else if (file[i] == '.') {
        i++;

        if (file[i] == '.') {
          i++;
        } else {
          SCANNER_ERROR(".");
        }
      }
    } else if (file[i] == '\'') {
      kind = CONSTANT;
      i = scan_c_char_seq(file, i);
    } else if (file[i] == '\"') {
      kind = STRING;
      i = scan_s_char_seq(file, i);
    } else if (file[i] == '[' || file[i] == ']' || file[i] == '(' ||
               file[i] == ')' || file[i] == '{' || file[i] == '}' ||
               file[i] == '~' || file[i] == '?' || file[i] == ';' ||
               file[i] == ',') {
      i++;
    } else if (file[i] == '-') {
      i++;

      if (file[i] == '>' || file[i] == '-' || file[i] == '=') {
        i++;
      }
    } else if (file[i] == '+') {
      i++;

      if (file[i] == '+' || file[i] == '=') {
        i++;
      }
    } else if (file[i] == '<') {
      i++;

      if (file[i] == '<') {
        i++;

        if (file[i] == '=') {
          i++;
        }
      } else if (file[i] == '=' || file[i] == ':' || file[i] == '%') {
        i++;
      }
    } else if (file[i] == '>') {
      i++;

      if (file[i] == '>') {
        i++;

        if (file[i] == '=') {
          i++;
        }
      } else if (file[i] == '=') {
        i++;
      }
    } else if (file[i] == '=') {
      i++;

      if (file[i] == '=') {
        i++;
      }
    } else if (file[i] == '!') {
      i++;

      if (file[i] == '=') {
        i++;
      }
    } else if (file[i] == '&') {
      i++;

      if (file[i] == '=' || file[i] == '&') {
        i++;
      }
    } else if (file[i] == '|') {
      i++;

      if (file[i] == '=' || file[i] == '|') {
        i++;
      }
    } else if (file[i] == ':') {
      i++;

      if (file[i] == ':' || file[i] == '>') {
        i++;
      }
    } else if (file[i] == '*') {
      i++;

      if (file[i] == '=') {
        i++;
      }
    } else if (file[i] == '%') {
      i++;

      if (file[i] == '=' || file[i] == '>') {
        i++;
      } else if (file[i] == ':') {
        i++;

        if (file[i] == '%') {
          i++;

          if (file[i] == ':') {
            i++;
          } else {
            SCANNER_ERROR(":");
          }
        }
      }
    } else if (file[i] == '^') {
      i++;

      if (file[i] == '=') {
        i++;
      }
    } else if (file[i] == '#') {
      i++;

      if (file[i] == '#') {
        i++;
      }
    } else {
      size_t lineno = seen_newlines + 1;
      size_t column = start - last_newline;
      ERRORV("scanner", "Unexpected character %c at %zu:%zu", file[i], lineno,
             column);
    }

    const char *value_begin = &file[start];
    size_t value_length = i - start;

    if (kind == IDENTIFIER &&
        is_in_array(value_begin, value_length, keywords, NELEMS(keywords))) {
      kind = KEYWORD;
    } else if (kind == IDENTIFIER &&
               is_in_array(value_begin, value_length, predefined_constants,
                           NELEMS(predefined_constants))) {
      kind = CONSTANT;
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
