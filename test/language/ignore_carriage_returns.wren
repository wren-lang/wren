// Sprinkle some carriage returns to ensure they are ignored anywhere:
IO.print("one")
IO.print("two")

// Generate a compile error so we can test that the line number is correct.
+ * // expect error
