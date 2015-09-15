System.print(123 == 123)  // expect: true
System.print(123 == 124)  // expect: false
System.print(-3 == 3)     // expect: false
System.print(0 == -0)     // expect: true

// Not equal to other types.
System.print(123 == "123") // expect: false
System.print(1 == true)    // expect: false
System.print(0 == false)   // expect: false

System.print(123 != 123)  // expect: false
System.print(123 != 124)  // expect: true
System.print(-3 != 3)     // expect: true
System.print(0 != -0)     // expect: false

// Not equal to other types.
System.print(123 != "123") // expect: true
System.print(1 != true)    // expect: true
System.print(0 != false)   // expect: true
