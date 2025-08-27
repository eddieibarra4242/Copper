#include "preprocess.h"
#include "log.h"
#include "pp_parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct define_macro {
  Token *id;
  struct token_span *replace_list;
  Span span;

  struct define_macro *next;
};

struct replace_record {
  Span macro_call;
  struct define_macro *macro;
  struct replace_record *next;
};

struct define_macro *list = NULL;
struct replace_record *replacements = NULL;

typedef struct file {
  const char *filename;
  Token *tokens;

  struct file *next;
} File;

#define NULL_CHECK(ptr)                                                        \
  do {                                                                         \
    if ((ptr) == NULL) {                                                       \
      CRITICAL("preprocessor", "Out of memory!");                              \
    }                                                                          \
  } while (0)

bool is_coord_less_than(Coord a, Coord b) {
  if (a.line_number < b.line_number) {
    return true;
  }

  if (a.line_number != b.line_number) {
    return false;
  }

  return a.column < b.column;
}

bool is_token_contained(Token *token, Span span) {
  if (is_coord_less_than(token->span.start, span.start) ||
      is_coord_less_than(span.end, token->span.start)) {
    return false; // token->span.start is outside the range
  }

  if (is_coord_less_than(token->span.end, span.start) ||
      is_coord_less_than(span.end, token->span.end)) {
    return false; // token->span.end is outside the range
  }

  return true;
}

bool is_token_pure(Token *token) {
  if (token->kind == NEWLINE) {
    return false; // newlines are only used for preprocessing
  }

  for (struct define_macro *macro = list; macro != NULL; macro = macro->next) {
    if (is_token_contained(token, macro->span)) {
      return false;
    }
  }

  return true;
};

File load_file(const char *filename) {
  File file = {0};
  file.filename = filename;

  FILE *input = fopen(filename, "r");

  if (!input) {
    CRITICAL("preprocessor", "Failed to open input file!");
  }

  TRY(fseek(input, 0, SEEK_END));
  long file_size = ftell(input);

  CHECK(file_size);

  TRY(fseek(input, 0, SEEK_SET));

  char *file_contents = malloc(file_size + 1);
  if (!file_contents) {
    ERROR("file", "Out of memory!");
  }

  size_t read_len = fread(file_contents, sizeof(char), file_size, input);

  if (read_len != (size_t)file_size) {
    // FIXME: do not fail here...
    ERROR("read", "Failed to read whole file!");
  }

  file_contents[file_size] = '\0';

  TRY(fclose(input));

  file.tokens = scan(filename, file_contents);
  free(file_contents);

  return file;
}

Token *preprocess(const char *filename) {
  File file = load_file(filename);

  init_pp_parser(file.tokens);
  pp_parse();

  Token *result = copy_token_list(file.tokens);
  Token *prev = NULL;
  for (Token *cur = result; cur != NULL;) {
    if (!is_token_pure(cur)) {
      if (prev) {
        prev->next = cur->next;
      } else {
        result = cur->next;
      }

      free_token(cur);
      cur = prev ? prev->next : result;
      continue;
    }

    prev = cur;
    cur = cur->next;
  }

  for (struct replace_record *record = replacements; record != NULL;
       record = record->next) {
    bool previous_cond = false;
    Token *prev = NULL;
    for (Token *cur = result; cur != NULL; cur = cur->next) {
      bool current_cond = is_token_contained(cur, record->macro_call);
      if (!current_cond) {
        previous_cond = false;
        prev = cur;
        continue;
      }

      if (previous_cond) {
        // already replaced this token
        if (prev) {
          prev->next = cur->next;
        } else {
          result = cur->next;
        }

        free_token(cur);
        cur = prev ? prev->next : result;
        continue;
      }

      Token *replacement = copy_token_span(record->macro->replace_list);
      NULL_CHECK(replacement);
      if (prev) {
        prev->next = replacement;
      } else {
        result = replacement;
      }

      Token *last = replacement;
      while (last->next) {
        last = last->next;
      }
      last->next = cur->next;
      free_token(cur);
      cur = prev ? prev->next : result;

      previous_cond = current_cond;
    }
  }

  return result;
}

void define(Token *id, struct token_span *replace_list, Token *start,
            Token *end) {
  struct define_macro *macro = malloc(sizeof(struct define_macro));
  NULL_CHECK(macro);

  macro->id = id;
  macro->replace_list = replace_list;
  macro->span.filename = start->span.filename;
  macro->span.start = start->span.start;
  macro->span.end = end->span.end;

  macro->next = list;
  list = macro;
}

struct token_span *create_token_span(Token *start) {
  struct token_span *span = malloc(sizeof(struct token_span));
  NULL_CHECK(span);

  span->start = start;
  span->end = start;

  return span;
}

struct token_span *enlarge_token_span(struct token_span *span, Token *new_end) {
  span->end = new_end;
  return span;
}

struct define_macro *find_macro(Token *id) {
  for (struct define_macro *macro = list; macro != NULL; macro = macro->next) {
    if (macro->id->length == id->length &&
        strncmp(macro->id->data, id->data, id->length) == 0) {
      return macro;
    }
  }

  return NULL;
}

void record_replacement(struct token_span *span, struct define_macro *macro) {
  struct replace_record *record = malloc(sizeof(struct replace_record));
  NULL_CHECK(record);

  record->macro_call.filename = span->start->span.filename;
  record->macro_call.start = span->start->span.start;
  record->macro_call.end = span->end->span.end;
  record->macro = macro;

  record->next = replacements;
  replacements = record;
}
