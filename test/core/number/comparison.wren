System.print(1 < 2)    // expect: true
System.print(2 < 2)    // expect: false
System.print(2 < 1)    // expect: false

System.print(1 <= 2)    // expect: true
System.print(2 <= 2)    // expect: true
System.print(2 <= 1)    // expect: false

System.print(1 > 2)    // expect: false
System.print(2 > 2)    // expect: false
System.print(2 > 1)    // expect: true

System.print(1 >= 2)    // expect: false
System.print(2 >= 2)    // expect: true
System.print(2 >= 1)    // expect: true

// Zero and negative zero compare the same.
System.print(0 < -0) // expect: false
System.print(-0 < 0) // expect: false
System.print(0 > -0) // expect: false
System.print(-0 > 0) // expect: false
System.print(0 <= -0) // expect: true
System.print(-0 <= 0) // expect: true
System.print(0 >= -0) // expect: true
System.print(-0 >= 0) // expect: true

// TODO: Wrong type for RHS.
