io.write("".contains(""))   // expect: 1
io.write("anything".contains("")) // expect: 1
io.write("something".contains("meth")) // expect: 1
io.write("something".contains("some")) // expect: 1
io.write("something".contains("ing")) // expect: 1
io.write("something".contains("math")) // expect: 0

// TODO(bob): Passing non-string as argument.
