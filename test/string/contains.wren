io.write("".contains(""))              // expect: true
io.write("anything".contains(""))      // expect: true
io.write("something".contains("meth")) // expect: true
io.write("something".contains("some")) // expect: true
io.write("something".contains("ing"))  // expect: true
io.write("something".contains("math")) // expect: false

// TODO(bob): Passing non-string as argument.
