// Note: These tests implicitly depend on ints being truthy.
//       Also rely on io.write() returning its argument.

// Return the first true argument.
io.write(1 || true) // expect: 1
io.write(false || 1) // expect: 1
io.write(false || false || true) // expect: true

// Return the last argument if all are false.
io.write(false || false) // expect: false
io.write(false || false || false) // expect: false

// Short-circuit at the first true argument.
io.write(false) || // expect: false
    io.write(true) || // expect: true
    io.write(true) // should not print

// Swallow a trailing newline.
io.write(true ||
    true) // expect: true

// Only false is falsy.
io.write(0 || true) // expect: 0
io.write(null || true) // expect: null
io.write(("" || true) == "") // expect: true
io.write(false || true) // expect: true
