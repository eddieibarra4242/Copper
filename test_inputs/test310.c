struct test {
  int a;
};

int main() {
  // Shift-reduce conflict.
  // This needs to match `sizeof unary_expression`!
  return sizeof (struct test) { .a = 12 };
}