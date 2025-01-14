#include <stdio.h>

#include "common.h"
#include "error.h"

int main(int args, char** argv) {
  UNUSED(argv);
  if (args < 2) {
    CRITICAL("CLI", "No input file!");
  }

  return 0;
}
