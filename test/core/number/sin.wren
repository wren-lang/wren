System.print(0.sin)             // expect: 0
System.print((Num.pi / 2).sin)  // expect: 1

// these should of course be 0, but it's not that precise
System.print(Num.pi.sin)        // expect: 1.2246467991474e-16
System.print((2 * Num.pi).sin)  // expect: -2.4492935982947e-16
