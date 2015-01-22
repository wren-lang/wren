IO.print("".contains(""))              // expect: true
IO.print("anything".contains(""))      // expect: true
IO.print("something".contains("meth")) // expect: true
IO.print("something".contains("some")) // expect: true
IO.print("something".contains("ing"))  // expect: true
IO.print("something".contains("math")) // expect: false

// Non-ASCII.
IO.print("søméthîng".contains("méth"))  // expect: true
IO.print("søméthîng".contains("meth")) // expect: false
