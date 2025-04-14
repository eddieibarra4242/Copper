int collatz(int x) {
  long i;
  for (i = 0; i < 100 && x != 1; i++) {
    int isOdd = x % 2;

    if (isOdd) {
      x = 3 * x + 1;
    } else {
      x = x / 2;
    }
  }

  return i;
}

int main() {
  return collatz(12);
}
