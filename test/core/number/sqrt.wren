System.print(4.sqrt)        // expect: 2
System.print(1000000.sqrt)  // expect: 1000
System.print(1.sqrt)        // expect: 1
System.print((-0).sqrt)     // expect: -0
System.print(0.sqrt)        // expect: 0
System.print(2.sqrt)        // expect: 1.4142135623731

System.print((-4).sqrt.isNan) // expect: true

// TODO: Tests for sin and cos.
