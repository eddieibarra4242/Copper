struct test {
  int a;
};

int main() {
  int a = 2;
  int b = 3;

  // Shift-reduce conflict. Both need to be satisfied!

  long shift = a < b ? (long)b : (long)a;
  struct test reduce = a < b ? (struct test){.a = a} : (struct test){.a = b};
  return shift > reduce.a ? 0 : 1;
}