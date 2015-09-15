// Note: These tests implicitly depend on ints being truthy.
//       Also rely on System.print() returning its argument.

// Return the first true argument.
System.print(1 || true) // expect: 1
System.print(false || 1) // expect: 1
System.print(false || false || true) // expect: true

// Return the last argument if all are false.
System.print(false || false) // expect: false
System.print(false || false || false) // expect: false

// Short-circuit at the first true argument.
System.print(false) || // expect: false
    System.print(true) || // expect: true
    System.print(true) // should not print

// Swallow a trailing newline.
System.print(true ||
    true) // expect: true
