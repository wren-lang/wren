
// * has higher precedence than +.
io.write(2 + 3 * 4) // expect: 14

// * has higher precedence than -.
io.write(20 - 3 * 4) // expect: 8

// Using () for grouping.
io.write((2 * (6 - (2 + 2)))) // expect: 4
