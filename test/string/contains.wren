IO.print("".contains(""))              // expect: true
IO.print("anything".contains(""))      // expect: true
IO.print("something".contains("meth")) // expect: true
IO.print("something".contains("some")) // expect: true
IO.print("something".contains("ing"))  // expect: true
IO.print("something".contains("math")) // expect: false

// Non-ASCII.
IO.print("søméthîng".contains("méth"))  // expect: true
IO.print("søméthîng".contains("meth")) // expect: false

// 8-bit clean
IO.print("a\0b\0c".contains("\0")) // expect: true
IO.print("a\0b\0c".contains("b")) // expect: true
IO.print("a\0b\0c".contains("b\0c")) // expect: true
IO.print("a\0b\0c".contains("bc")) // expect: false
