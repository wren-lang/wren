IO.print(Num == Num)  // expect: true
IO.print(Num == Bool) // expect: false

// Not equal to other types.
IO.print(Num == 123)  // expect: false
IO.print(Num == true) // expect: false

IO.print(Num != Num)  // expect: false
IO.print(Num != Bool) // expect: true

// Not equal to other types.
IO.print(Num != 123)  // expect: true
IO.print(Num != true) // expect: true
