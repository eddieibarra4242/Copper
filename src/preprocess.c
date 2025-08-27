#include "preprocess.h"
#include "log.h"
#include "pp_parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

struct define_macro {
  Token *id;
  struct token_list *replace_list;
  Token *start;
  Token *end;

  struct define_macro *next;
};

struct define_macro *list = NULL;

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

bool is_token_contained(Token *token, Token *start, Token *end) {
  Coord start_coord = start->span.start;
  Coord end_coord = end->span.end;

  if (is_coord_less_than(token->span.start, start_coord) ||
      is_coord_less_than(end_coord, token->span.start)) {
    return false; // token->span.start is outside the range
  }

  if (is_coord_less_than(token->span.end, start_coord) ||
      is_coord_less_than(end_coord, token->span.end)) {
    return false; // token->span.end is outside the range
  }

  return true;
}

bool is_token_pure(Token *token) {
  if (token->kind == NEWLINE) {
    return false; // newlines are only used for preprocessing
  }

  for (struct define_macro *macro = list; macro != NULL; macro = macro->next) {
    if (is_token_contained(token, macro->start, macro->end)) {
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

  return result;
}

void define(Token *id, struct token_list *replace_list, Token *start,
            Token *end) {
  struct define_macro *macro = malloc(sizeof(struct define_macro));
  NULL_CHECK(macro);

  macro->id = id;
  macro->replace_list = replace_list;
  macro->start = start;
  macro->end = end;

  macro->next = list;
  list = macro;
}

struct token_list *create_token_list(Token *start) {
  struct token_list *list = malloc(sizeof(struct token_list));
  NULL_CHECK(list);

  list->start = start;
  list->end = start;

  return list;
}

struct token_list *enlarge_token_list(struct token_list *list, Token *new_end) {
  list->end = new_end;
  return list;
}
