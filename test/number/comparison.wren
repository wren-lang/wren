io.write(1 < 2)    // expect: true
io.write(2 < 2)    // expect: false
io.write(2 < 1)    // expect: false

io.write(1 <= 2)    // expect: true
io.write(2 <= 2)    // expect: true
io.write(2 <= 1)    // expect: false

io.write(1 > 2)    // expect: false
io.write(2 > 2)    // expect: false
io.write(2 > 1)    // expect: true

io.write(1 >= 2)    // expect: false
io.write(2 >= 2)    // expect: true
io.write(2 >= 1)    // expect: true

// Zero and negative zero compare the same.
io.write(0 < -0) // expect: false
io.write(-0 < 0) // expect: false
io.write(0 > -0) // expect: false
io.write(-0 > 0) // expect: false
io.write(0 <= -0) // expect: true
io.write(-0 <= 0) // expect: true
io.write(0 >= -0) // expect: true
io.write(-0 >= 0) // expect: true

// TODO: Wrong type for RHS.
