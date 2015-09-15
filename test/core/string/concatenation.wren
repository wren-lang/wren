System.print("a" + "b") // expect: ab

// 8-bit clean.
System.print(("a\0b" + "\0c") == "a\0b\0c")  // expect: true
