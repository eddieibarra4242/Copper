int collatz(int x) {
  for (long i = 0; i < 100 && x != 1; i++) {
    int isOdd = x % 2;

    if (isOdd) {
      x = 3 * x + 1;
    } else {
      x = x / 2;
    }
  }

  return x;
}

int main() {
  return collatz(12);
}
