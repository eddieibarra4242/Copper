#include <dirent.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <wait.h>

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

    printf("\nRunning %s\n", entry->d_name);
    int status = run_cu(entry->d_name);

    if (status != 0) {
      printf("Failed to run `%s`\n", path);
      return 1;
    }
  }

  closedir(dir);
}