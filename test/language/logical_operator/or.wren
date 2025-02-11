// Note: These tests implicitly depend on ints being truthy.
//       Also rely on System.print() returning its argument.

// Return the first truthy argument.
System.print(true || 2) // expect: true
System.print(false || true) // expect: true
System.print(false || null || true) // expect: true

// Return the last argument if all are falsy.
System.print(null || false) // expect: false
System.print(false || false || null) // expect: null

// Short-circuit at the first truthy argument.
System.print(false) || // expect: false
    System.print(true) || // expect: true
    System.print(true) // should not print

// Swallow a trailing newline.
System.print(true ||
    true) // expect: true
