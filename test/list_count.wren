io.write([].count)           // expect: 0
io.write([1].count)          // expect: 1
io.write([1, 2, 3, 4].count) // expect: 4

// TODO(bob): Literal syntax, including newline handling.
// TODO(bob): Unterminated list literal.
// TODO(bob): Subscript operator.
