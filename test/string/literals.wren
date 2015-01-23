IO.print("".count)   // expect: 0
IO.print("a string") // expect: a string

// Non-ASCII.
IO.print("A~¶Þॐஃ") // expect: A~¶Þॐஃ
