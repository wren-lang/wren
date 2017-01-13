System.print(0.sin)             // expect: 0
System.print((Num.pi / 2).sin)  // expect: 1

// these should of course be 0, but it's not that precise
System.print(Num.pi.sin.abs < 1.0e-15)        // expect: true
System.print((2 * Num.pi).sin.abs < 1.0e-15)  // expect: true
