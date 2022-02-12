// Bool
System.print(true ~~ true)    // expect: true
System.print(true ~~ false)   // expect: false
System.print(false ~~ true)   // expect: false
System.print(false ~~ false)  // expect: true

// Null
System.print(null ~~ null)        // expect: true
System.print(1 ~~ null)           // expect: false
System.print("not null" ~~ null)  // expect: false

// Num
System.print(1 ~~ 1)    // expect: true
System.print(1 ~~ 1.0)  // expect: true
System.print(0 ~~ 0)    // expect: true
System.print(-0 ~~ 0)   // expect: true
System.print(0 ~~ -0)   // expect: true
System.print(1 ~~ 2)    // expect: false

// String
System.print("" ~~ "")      // expect: true
System.print("\n" ~~ "\n")  // expect: true
System.print("a" ~~ "a")    // expect: true
System.print("a" ~~ "a ")   // expect: false
System.print("a" ~~ "bar")  // expect: false
System.print("bar" ~~ "a")  // expect: false
