System.print(123.truncate)      // expect: 123
System.print((-123).truncate)   // expect: -123
System.print(0.truncate)        // expect: 0
System.print((-0).truncate)     // expect: -0
System.print(0.123.truncate)    // expect: 0
System.print(12.3.truncate)     // expect: 12
System.print((-0.123).truncate) // expect: -0
System.print((-12.3).truncate)  // expect: -12

// Using 32-bit representation, values "beyond" those  two will lead to
// approximation.
System.print((12345678901234.5).truncate)   // expect: 12345678901234
System.print((-12345678901234.5).truncate)  // expect: -12345678901234
