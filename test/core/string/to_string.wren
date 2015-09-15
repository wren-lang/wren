System.print("".toString == "") // expect: true
System.print("blah".toString == "blah") // expect: true

// 8-bit clean.
System.print("a\0b\0c".toString == "a\0b\0c") // expect: true
System.print("a\0b\0c".toString == "a") // expect: false
System.print("a\0b\0c".toString) // expect: a
