typedef int x;

// x can refer to the type x above, but it must be parsed as a direct declarator.
int main(int x) {
  return 1;
}