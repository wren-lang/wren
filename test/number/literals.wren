IO.write(123)     // expect: 123
IO.write(987654)  // expect: 987654
IO.write(0)       // expect: 0
IO.write(-0)      // expect: -0

IO.write(123.456) // expect: 123.456
IO.write(-0.001)  // expect: -0.001

// TODO: Hex? Scientific notation?
// TODO: Literals at and beyond numeric limits.
