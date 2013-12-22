IO.write("" == "")          // expect: true
IO.write("abcd" == "abcd")  // expect: true
IO.write("abcd" == "d")     // expect: false
IO.write("e" == "abcd")     // expect: false
IO.write("" == "abcd")      // expect: false

// Not equal to other types.
IO.write("1" == 1)        // expect: false
IO.write("true" == true)  // expect: false

IO.write("" != "")          // expect: false
IO.write("abcd" != "abcd")  // expect: false
IO.write("abcd" != "d")     // expect: true
IO.write("e" != "abcd")     // expect: true
IO.write("" != "abcd")      // expect: true

// Not equal to other types.
IO.write("1" != 1)        // expect: true
IO.write("true" != true)  // expect: true
