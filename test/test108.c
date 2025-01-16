// Dangling else problem
// This syntax should cause a shift-reduce conflict in our grammar.
// Our parser needs to shift the else token instead of reducing the inner
// if statement.
int main() {
  if (12)
    if (3)
      return 0;
  else
    // This else belongs to the inner if statement
    return 1;
}