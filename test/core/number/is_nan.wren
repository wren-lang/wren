System.print(1.isNan)      // expect: false
System.print((0/0).isNan)  // expect: true

// Infinity is not NaN.
System.print((1/0).isNan)  // expect: false
