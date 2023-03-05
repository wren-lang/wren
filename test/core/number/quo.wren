System.print(10.quo(4))        // expect: 2
System.print(10.quo(-4))       // expect: -2
System.print((-10).quo(4))     // expect: -2
System.print((-10).quo(-4))    // expect: 2
System.print(15.4.quo(3.6))    // expect: 4
System.print(15.4.quo(-3.6))   // expect: -4

// Divide by zero.
System.print(3.quo(0))         // expect: infinity
System.print((-3).quo(0))      // expect: -infinity
System.print(0.quo(0))         // expect: nan
System.print((-0).quo(0))      // expect: nan
System.print(3.quo(-0))        // expect: -infinity
System.print((-3).quo(-0))     // expect: infinity
System.print(0.quo(-0))        // expect: nan
System.print((-0).quo(-0))     // expect: nan
