// Note: These tests implicitly depend on ints being truthy.
//       Also rely on System.print() returning its argument.

// Return the first non-true argument.
System.print(false && 1) // expect: false
System.print(true && 1) // expect: 1
System.print(1 && 2 && false) // expect: false

// Return the last argument if all are true.
System.print(1 && true) // expect: true
System.print(1 && 2 && 3) // expect: 3

// Short-circuit at the first false argument.
System.print(true) && // expect: true
    System.print(false) && // expect: false
    System.print(false) // should not print

// Swallow a trailing newline.
System.print(true &&
    true) // expect: true
