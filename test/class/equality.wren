IO.write(Num == Num)  // expect: true
IO.write(Num == Bool) // expect: false

// Not equal to other types.
IO.write(Num == 123)  // expect: false
IO.write(Num == true) // expect: false

IO.write(Num != Num)  // expect: false
IO.write(Num != Bool) // expect: true

// Not equal to other types.
IO.write(Num != 123)  // expect: true
IO.write(Num != true) // expect: true
