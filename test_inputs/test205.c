// This file cannot be parsed with a context free parser.

typedef unsigned long size_t;

int bop(long x) {
  return x + 1;
}

int main() {
  int a = 3;
  int b = 1;
  int c = (bop)((long)a - b);

  return (size_t)((long)a - c);
}