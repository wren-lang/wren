// * has higher precedence than +.
System.print(2 + 3 * 4) // expect: 14

// * has higher precedence than -.
System.print(20 - 3 * 4) // expect: 8

// / has higher precedence than +.
System.print(2 + 6 / 3) // expect: 4

// / has higher precedence than -.
System.print(2 - 6 / 3) // expect: 0

// < has higher precedence than ==.
System.print(false == 2 < 1) // expect: true

// > has higher precedence than ==.
System.print(false == 1 > 2) // expect: true

// <= has higher precedence than ==.
System.print(false == 2 <= 1) // expect: true

// >= has higher precedence than ==.
System.print(false == 1 >= 2) // expect: true

// is has higher precedence than ==.
System.print(true == 10 is Num) // expect: true
System.print(10 is Num == false) // expect: false

// Unary - has lower precedence than ..
System.print(-"abc".count) // expect: -3

// 1 - 1 is not space-sensitive.
System.print(1 - 1) // expect: 0
System.print(1 -1)  // expect: 0
System.print(1- 1)  // expect: 0
System.print(1-1)   // expect: 0

// TODO: %, associativity.

// Using () for grouping.
System.print((2 * (6 - (2 + 2)))) // expect: 4
