IO.print(5 % 3) // expect: 2
IO.print(10 % 5) // expect: 0
IO.print(-4 % 3) // expect: -1
IO.print(4 % -3) // expect: 1
IO.print(-4 % -3) // expect: -1
IO.print(-4.2 % 3.1) // expect: -1.1
IO.print(4.2 % -3.1) // expect: 1.1
IO.print(-4.2 % -3.1) // expect: -1.1

// Left associative.
IO.print(13 % 7 % 4) // expect: 2

// Precedence.
IO.print(13 + 1 % 7) // expect: 14

// TODO: Unsupported RHS types.
// TODO: Error on mod by zero.
