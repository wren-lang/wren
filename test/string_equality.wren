io.write("" == "")          // expect: true
io.write("abcd" == "abcd")  // expect: true
io.write("abcd" == "d")     // expect: false
io.write("e" == "abcd")     // expect: false
io.write("" == "abcd")      // expect: false

// Not equal to other types.
io.write("1" == 1)        // expect: false
io.write("true" == true)  // expect: false

io.write("" != "")          // expect: false
io.write("abcd" != "abcd")  // expect: false
io.write("abcd" != "d")     // expect: true
io.write("e" != "abcd")     // expect: true
io.write("" != "abcd")      // expect: true

// Not equal to other types.
io.write("1" != 1)        // expect: true
io.write("true" != true)  // expect: true
