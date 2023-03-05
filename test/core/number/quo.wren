System.print((10/4).truncate)        // expect: 2
System.print((10/(-4)).truncate)     // expect: -2
System.print(((-10)/(4)).truncate)   // expect: -2
System.print(((-10)/(-4)).truncate)  // expect: 2
System.print((15.4/3.6).truncate)    // expect: 4
System.print((15.4/-(3.6)).truncate) // expect: -4

// Divide by zero.
System.print((3/0).truncate)         // expect: infinity
System.print(((-3)/0).truncate)      // expect: -infinity
System.print((0/0).truncate)         // expect: nan
System.print(((-0)/0).truncate)      // expect: nan
System.print((3/(-0)).truncate)      // expect: -infinity
System.print(((-3)/(-0)).truncate)   // expect: infinity
System.print((0/(-0)).truncate)      // expect: nan
System.print(((-0)/(-0)).truncate)   // expect: nan
