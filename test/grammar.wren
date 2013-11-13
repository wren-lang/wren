
// * has higher precedence than +.
io.write(2 + 3 * 4) // expect: 14

// * has higher precedence than -.
io.write(20 - 3 * 4) // expect: 8

// / has higher precedence than +.
io.write(2 + 6 / 3) // expect: 4

// / has higher precedence than -.
io.write(2 - 6 / 3) // expect: 0

// < has higher precedence than ==.
io.write(false == 2 < 1) // expect: true

// > has higher precedence than ==.
io.write(false == 1 > 2) // expect: true

// <= has higher precedence than ==.
io.write(false == 2 <= 1) // expect: true

// >= has higher precedence than ==.
io.write(false == 1 >= 2) // expect: true

// Unary - has lower precedence than ..
io.write(-"abc".count) // expect: -3

// TODO(bob): %, associativity.

// Using () for grouping.
io.write((2 * (6 - (2 + 2)))) // expect: 4
