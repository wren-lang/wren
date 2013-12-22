// * has higher precedence than +.
IO.write(2 + 3 * 4) // expect: 14

// * has higher precedence than -.
IO.write(20 - 3 * 4) // expect: 8

// / has higher precedence than +.
IO.write(2 + 6 / 3) // expect: 4

// / has higher precedence than -.
IO.write(2 - 6 / 3) // expect: 0

// < has higher precedence than ==.
IO.write(false == 2 < 1) // expect: true

// > has higher precedence than ==.
IO.write(false == 1 > 2) // expect: true

// <= has higher precedence than ==.
IO.write(false == 2 <= 1) // expect: true

// >= has higher precedence than ==.
IO.write(false == 1 >= 2) // expect: true

// Unary - has lower precedence than ..
IO.write(-"abc".count) // expect: -3

// TODO: %, associativity.

// Using () for grouping.
IO.write((2 * (6 - (2 + 2)))) // expect: 4
