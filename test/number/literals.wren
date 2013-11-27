io.write(123)     // expect: 123
io.write(987654)  // expect: 987654
io.write(0)       // expect: 0
io.write(-0)      // expect: -0

io.write(123.456) // expect: 123.456
io.write(-0.001)  // expect: -0.001

// TODO(bob): Hex? Scientific notation?
// TODO(bob): Literals at and beyond numeric limits.
