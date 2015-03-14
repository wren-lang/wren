// One byte UTF-8 Sequences.
IO.print("\u0041")     // expect: A
IO.print("\u007e")     // expect: ~

// Two byte sequences.
IO.print("\u00b6")     // expect: ¶
IO.print("\u00de")     // expect: Þ

// Three byte sequences.
IO.print("\u0950")     // expect: ॐ
IO.print("\u0b83")     // expect: ஃ

// Capitalized hex.
IO.print("\u00B6")     // expect: ¶
IO.print("\u00DE")     // expect: Þ

// TODO: Syntax for Unicode escapes > 0xffff?
