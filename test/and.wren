// Note: These tests implicitly depend on ints being truthy.
//       Also rely on IO.write() returning its argument.

// Return the first non-true argument.
IO.write(false && 1) // expect: false
IO.write(true && 1) // expect: 1
IO.write(1 && 2 && false) // expect: false

// Return the last argument if all are true.
IO.write(1 && true) // expect: true
IO.write(1 && 2 && 3) // expect: 3

// Short-circuit at the first false argument.
IO.write(true) && // expect: true
    IO.write(false) && // expect: false
    IO.write(false) // should not print

// Swallow a trailing newline.
IO.write(true &&
    true) // expect: true

// Only false is falsy.
IO.write(0 && true) // expect: true
IO.write(null && true) // expect: true
IO.write("" && true) // expect: true
IO.write(false && true) // expect: false
