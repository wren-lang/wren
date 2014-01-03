IO.write("".contains(""))              // expect: true
IO.write("anything".contains(""))      // expect: true
IO.write("something".contains("meth")) // expect: true
IO.write("something".contains("some")) // expect: true
IO.write("something".contains("ing"))  // expect: true
IO.write("something".contains("math")) // expect: false
