IO.print("" == "")          // expect: true
IO.print("abcd" == "abcd")  // expect: true
IO.print("abcd" == "d")     // expect: false
IO.print("e" == "abcd")     // expect: false
IO.print("" == "abcd")      // expect: false

// Not equal to other types.
IO.print("1" == 1)        // expect: false
IO.print("true" == true)  // expect: false

IO.print("" != "")          // expect: false
IO.print("abcd" != "abcd")  // expect: false
IO.print("abcd" != "d")     // expect: true
IO.print("e" != "abcd")     // expect: true
IO.print("" != "abcd")      // expect: true

// Not equal to other types.
IO.print("1" != 1)        // expect: true
IO.print("true" != true)  // expect: true
