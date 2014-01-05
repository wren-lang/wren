IO.print(123 == 123)  // expect: true
IO.print(123 == 124)  // expect: false
IO.print(-3 == 3)     // expect: false
IO.print(0 == -0)     // expect: true

// Not equal to other types.
IO.print(123 == "123") // expect: false
IO.print(1 == true)    // expect: false
IO.print(0 == false)   // expect: false

IO.print(123 != 123)  // expect: false
IO.print(123 != 124)  // expect: true
IO.print(-3 != 3)     // expect: true
IO.print(0 != -0)     // expect: false

// Not equal to other types.
IO.print(123 != "123") // expect: true
IO.print(1 != true)    // expect: true
IO.print(0 != false)   // expect: true
