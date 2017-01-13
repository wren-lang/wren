System.print(0.cos)             // expect: 1
System.print(Num.pi.cos)        // expect: -1
System.print((2 * Num.pi).cos)  // expect: 1

// this should of course be 0, but it's not that precise
System.print((Num.pi / 2).cos.abs < 1.0e-16) // expect: true
