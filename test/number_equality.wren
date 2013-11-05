io.write(123 == 123)  // expect: true
io.write(123 == 124)  // expect: false
io.write(-3 == 3)     // expect: false

// Not equal to other types.
io.write(123 == "123") // expect: false
io.write(1 == true)    // expect: false
io.write(0 == false)   // expect: false

io.write(123 != 123)  // expect: false
io.write(123 != 124)  // expect: true
io.write(-3 != 3)     // expect: true

// Not equal to other types.
io.write(123 != "123") // expect: true
io.write(1 != true)    // expect: true
io.write(0 != false)   // expect: true

// TODO(bob): Should 0 == -0?