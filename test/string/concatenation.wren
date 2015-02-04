IO.print("a" + "b") // expect: ab

// 8-bit clean
IO.print(("a\0b" + "\0c") == "a\0b\0c")  // expect: true
