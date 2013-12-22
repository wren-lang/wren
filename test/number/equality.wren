IO.write(123 == 123)  // expect: true
IO.write(123 == 124)  // expect: false
IO.write(-3 == 3)     // expect: false
IO.write(0 == -0)     // expect: true

// Not equal to other types.
IO.write(123 == "123") // expect: false
IO.write(1 == true)    // expect: false
IO.write(0 == false)   // expect: false

IO.write(123 != 123)  // expect: false
IO.write(123 != 124)  // expect: true
IO.write(-3 != 3)     // expect: true
IO.write(0 != -0)     // expect: false

// Not equal to other types.
IO.write(123 != "123") // expect: true
IO.write(1 != true)    // expect: true
IO.write(0 != false)   // expect: true
