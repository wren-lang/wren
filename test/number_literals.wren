io.write(123)    // expect: 123
io.write(987654) // expect: 987654
io.write(0)      // expect: 0
io.write(-0)     // expect: 0

// TODO(bob): Floating point numbers.
