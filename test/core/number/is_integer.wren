System.print(123.isInteger) // expect: true
System.print(123.0.isInteger) // expect: true
System.print(0.isInteger) // expect: true
System.print(1.0000001.isInteger) // expect: false
System.print((-2.3).isInteger) // expect: false

// NaN is not an integer.
System.print((0/0).isInteger) // expect: false

// Infinity is not an integer.
System.print((1/0).isInteger)  // expect: false
