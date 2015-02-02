IO.print("".count)   // expect: 0
IO.print("a string".count) // expect: 8

// 8-bit clean
IO.print("\0".count)  // expect: 1
IO.print("a\0b".count)  // expect: 3
IO.print("\0c".count)  // expect: 2
IO.print(("a\0b" + "\0c").count)  // expect: 5
