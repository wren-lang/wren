IO.print(0.cos)             // expect: 1
IO.print(Num.pi.cos)        // expect: -1
IO.print((2 * Num.pi).cos)  // expect: 1

// this should of course be 0, but it's not that precise
IO.print((Num.pi / 2).cos)  // expect: 6.1232339957368e-17
