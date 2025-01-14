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

  printf("File size is %ld!\n", file_size);

  TRY(fclose(input));

  return 0;
}
