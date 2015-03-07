IO.print(123.truncate)      // expect: 123
IO.print((-123).truncate)   // expect: -123
IO.print(0.truncate)        // expect: 0
IO.print((-0).truncate)     // expect: -0
IO.print(0.123.truncate)    // expect: 0
IO.print(12.3.truncate)     // expect: 12
IO.print((-0.123).truncate) // expect: -0
IO.print((-12.3).truncate)  // expect: -12

// Using 32-bit representation, values "beyond" those  two will lead to
// approximation.
IO.print((12345678901234.5).truncate)   // expect: 12345678901234
IO.print((-12345678901234.5).truncate)  // expect: -12345678901234
