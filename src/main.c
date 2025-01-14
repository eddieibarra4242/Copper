#include <stdio.h>

#include "common.h"
#include "error.h"

int main(int args, char** argv) {
  if (args < 2) {
    CRITICAL("cli", "No input file!");
  }

  FILE *input = fopen(argv[1], "r");

  if (!input) {
    CRITICAL("cli", "Failed to open input file!");
  }

  TRY(fseek(input, 0, SEEK_END));
  long file_size = ftell(input);

  CHECK(file_size);

  TRY(fseek(input, 0, SEEK_SET));

  char *file = malloc(file_size + 1);
  if (!file) {
    ERROR("file", "Out of memory!");
  }

  size_t read_len = fread(file, sizeof(char), file_size, input);

  if (read_len != (size_t)file_size) {
    // FIXME: do not fail here...
    ERROR("read", "Failed to read whole file!");
  }

  file[file_size] = '\0';

  TRY(fclose(input));

  printf("%s\n", file);

  free(file);

  return 0;
}
