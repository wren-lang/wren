IO.print("".toString == "") // expect: true
IO.print("blah".toString == "blah") // expect: true

// 8-bit clean.
IO.print("a\0b\0c".toString == "a\0b\0c") // expect: true
IO.print("a\0b\0c".toString == "a") // expect: false
IO.print("a\0b\0c".toString) // expect: a
