// Note: These tests implicitly depend on ints being truthy.
//       Also rely on IO.print() returning its argument.

// Return the first non-true argument.
IO.print(false && 1) // expect: false
IO.print(true && 1) // expect: 1
IO.print(1 && 2 && false) // expect: false

// Return the last argument if all are true.
IO.print(1 && true) // expect: true
IO.print(1 && 2 && 3) // expect: 3

// Short-circuit at the first false argument.
IO.print(true) && // expect: true
    IO.print(false) && // expect: false
    IO.print(false) // should not print

// Swallow a trailing newline.
IO.print(true &&
    true) // expect: true
