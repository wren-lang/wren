IO.print(123)     // expect: 123
IO.print(987654)  // expect: 987654
IO.print(0)       // expect: 0
IO.print(-0)      // expect: -0

IO.print(123.456) // expect: 123.456
IO.print(-0.001)  // expect: -0.001

// TODO: Scientific notation?
// TODO: Literals at and beyond numeric limits.
