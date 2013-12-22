// Note: These tests implicitly depend on ints being truthy.
//       Also rely on IO.write() returning its argument.

// Return the first true argument.
IO.write(1 || true) // expect: 1
IO.write(false || 1) // expect: 1
IO.write(false || false || true) // expect: true

// Return the last argument if all are false.
IO.write(false || false) // expect: false
IO.write(false || false || false) // expect: false

// Short-circuit at the first true argument.
IO.write(false) || // expect: false
    IO.write(true) || // expect: true
    IO.write(true) // should not print

// Swallow a trailing newline.
IO.write(true ||
    true) // expect: true

// Only false is falsy.
IO.write(0 || true) // expect: 0
IO.write(null || true) // expect: null
IO.write(("" || true) == "") // expect: true
IO.write(false || true) // expect: true
