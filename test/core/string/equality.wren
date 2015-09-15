System.print("" == "")          // expect: true
System.print("abcd" == "abcd")  // expect: true
System.print("abcd" == "d")     // expect: false
System.print("e" == "abcd")     // expect: false
System.print("" == "abcd")      // expect: false

// Not equal to other types.
System.print("1" == 1)        // expect: false
System.print("true" == true)  // expect: false

System.print("" != "")          // expect: false
System.print("abcd" != "abcd")  // expect: false
System.print("abcd" != "d")     // expect: true
System.print("e" != "abcd")     // expect: true
System.print("" != "abcd")      // expect: true

// Not equal to other types.
System.print("1" != 1)        // expect: true
System.print("true" != true)  // expect: true

// Non-ASCII.
System.print("vålue" == "value") // expect: false
System.print("vålue" == "vålue") // expect: true

// 8-bit clean.
System.print("a\0b\0c" == "a") // expect: false
System.print("a\0b\0c" == "abc") // expect: false
System.print("a\0b\0c" == "a\0b\0c") // expect: true
