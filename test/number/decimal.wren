IO.print(123.decimal)      // expect: 0
IO.print((-123).decimal)   // expect: 0
IO.print(0.decimal)        // expect: 0
IO.print((-0).decimal)     // expect: -0
IO.print(0.123.decimal)    // expect: 0.123
IO.print(12.3.decimal)     // expect: 0.3
IO.print((-0.123).decimal) // expect: -0.123
IO.print((-12.3).decimal)  // expect: -0.3
