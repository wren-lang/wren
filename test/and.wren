// Note: These tests implicitly depend on ints being truthy.
//       Also rely on io.write() returning its argument.

// Return the first non-true argument.
io.write(false && 1) // expect: false
io.write(true && 1) // expect: 1
io.write(1 && 2 && false) // expect: false

// Return the last argument if all are true.
io.write(1 && true) // expect: true
io.write(1 && 2 && 3) // expect: 3

// Short-circuit at the first false argument.
io.write(true) && // expect: true
    io.write(false) && // expect: false
    io.write(false) // should not print

// Swallow a trailing newline.
io.write(true &&
    true) // expect: true
