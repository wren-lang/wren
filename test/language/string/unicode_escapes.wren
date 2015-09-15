// One byte UTF-8 Sequences.
System.print("\u0041")     // expect: A
System.print("\u007e")     // expect: ~

// Two byte sequences.
System.print("\u00b6")     // expect: ¶
System.print("\u00de")     // expect: Þ

// Three byte sequences.
System.print("\u0950")     // expect: ॐ
System.print("\u0b83")     // expect: ஃ

// Capitalized hex.
System.print("\u00B6")     // expect: ¶
System.print("\u00DE")     // expect: Þ

// TODO: Syntax for Unicode escapes > 0xffff?
