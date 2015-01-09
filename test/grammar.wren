// * has higher precedence than +.
IO.print(2 + 3 * 4) // expect: 14

// * has higher precedence than -.
IO.print(20 - 3 * 4) // expect: 8

// / has higher precedence than +.
IO.print(2 + 6 / 3) // expect: 4

// / has higher precedence than -.
IO.print(2 - 6 / 3) // expect: 0

// < has higher precedence than ==.
IO.print(false == 2 < 1) // expect: true

// > has higher precedence than ==.
IO.print(false == 1 > 2) // expect: true

// <= has higher precedence than ==.
IO.print(false == 2 <= 1) // expect: true

// >= has higher precedence than ==.
IO.print(false == 1 >= 2) // expect: true

// Unary - has lower precedence than ..
IO.print(-"abc".count) // expect: -3

// 1 - 1 is not space-sensitive.
IO.print(1 - 1) // expect: 0
IO.print(1 -1)  // expect: 0
IO.print(1- 1)  // expect: 0
IO.print(1-1)   // expect: 0

// TODO: %, associativity.

// Using () for grouping.
IO.print((2 * (6 - (2 + 2)))) // expect: 4
