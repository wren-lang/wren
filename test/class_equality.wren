io.write(Num == Num)  // expect: true
io.write(Num == Bool) // expect: false

// Not equal to other types.
io.write(Num == 123)  // expect: false
io.write(Num == true) // expect: false

io.write(Num != Num)  // expect: false
io.write(Num != Bool) // expect: true

// Not equal to other types.
io.write(Num != 123)  // expect: true
io.write(Num != true) // expect: true
