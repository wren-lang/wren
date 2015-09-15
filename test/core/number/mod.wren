System.print(5 % 3) // expect: 2
System.print(10 % 5) // expect: 0
System.print(-4 % 3) // expect: -1
System.print(4 % -3) // expect: 1
System.print(-4 % -3) // expect: -1
System.print(-4.2 % 3.1) // expect: -1.1
System.print(4.2 % -3.1) // expect: 1.1
System.print(-4.2 % -3.1) // expect: -1.1

// Left associative.
System.print(13 % 7 % 4) // expect: 2

// Precedence.
System.print(13 + 1 % 7) // expect: 14

// TODO: Unsupported RHS types.
// TODO: Error on mod by zero.
