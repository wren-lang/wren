System.print("".contains(""))              // expect: true
System.print("anything".contains(""))      // expect: true
System.print("something".contains("meth")) // expect: true
System.print("something".contains("some")) // expect: true
System.print("something".contains("ing"))  // expect: true
System.print("something".contains("math")) // expect: false

// Non-ASCII.
System.print("søméthîng".contains("méth"))  // expect: true
System.print("søméthîng".contains("meth")) // expect: false

// 8-bit clean.
System.print("a\0b\0c".contains("\0")) // expect: true
System.print("a\0b\0c".contains("b")) // expect: true
System.print("a\0b\0c".contains("b\0c")) // expect: true
System.print("a\0b\0c".contains("bc")) // expect: false
