#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <wait.h>

const char *const SKIP_TESTS[] = {
  "test200.c", // symbol 'a' is not defined
  "test201.c", // symbol 'a' is not defined
  "test202.c", // symbol 'array' is not defined
  "test204.c", // symbol 'a' is not defined
  "test206.c", // unparseable file due to user-defined type being used as a
               // function name
  "test216.c", // `bop` is not defined
  "test310.c", // FIXME: sizeof not implemented
  "test311.c", // FIXME: braced initializers not implemented
  "test350.c", // FIXME: sizeof not implemented
  "test600.c", // FIXME: pre-processing not implemented (can't include stdio.h)
  "test601.c", // FIXME: pre-processing not implemented (can't include stdio.h)
  "test602.c", // has a syntax error (gcc agrees) TODO: check if this is a
               // valid C23 program
  "test900.c", // FIXME: pointers are not implemented
};

#define NUM_SKIP_TESTS (sizeof(SKIP_TESTS) / sizeof(SKIP_TESTS[0]))

char path[1024];

int run_cu(const char *input) {
  snprintf(path, 1024, "./test_inputs/%s", input);

  int child = fork();

  if (child == 0) {
    char *const args[] = {"./bin/cu", path, NULL};
    execv("./bin/cu", args);
    return 1;
  }

  int status;
  int dead_child = wait(&status);

  if (dead_child != child) {
    perror("Something went wrong!");
  }

  return status;
}

bool should_skip(const char *input) {
  for (size_t i = 0; i < NUM_SKIP_TESTS; i++) {
    if (strcmp(input, SKIP_TESTS[i]) == 0) {
      return true;
    }
  }
  return false;
}

int main() {
  DIR *dir;
  struct dirent *entry;

  dir = opendir("./test_inputs");

  if (dir == NULL) {
    perror("Unable to open directory!");
    return 1;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    printf("\nRunning %s...", entry->d_name);

    if (should_skip(entry->d_name)) {
      printf(" \033[0;34mSkipped\033[0m\n", entry->d_name);
      continue;
    }

    int status = run_cu(entry->d_name);

    if (status != 0) {
      printf("\033[0;31m Failed\033[0m\n", path);
    } else {
      printf("\033[0;32m Passed\033[0m\n");
    }
  }

  closedir(dir);
}