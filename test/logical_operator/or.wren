// Note: These tests implicitly depend on ints being truthy.
//       Also rely on IO.print() returning its argument.

// Return the first true argument.
IO.print(1 || true) // expect: 1
IO.print(false || 1) // expect: 1
IO.print(false || false || true) // expect: true

// Return the last argument if all are false.
IO.print(false || false) // expect: false
IO.print(false || false || false) // expect: false

// Short-circuit at the first true argument.
IO.print(false) || // expect: false
    IO.print(true) || // expect: true
    IO.print(true) // should not print

// Swallow a trailing newline.
IO.print(true ||
    true) // expect: true
