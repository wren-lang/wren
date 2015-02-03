IO.print("abcd".startsWith("cd")) // expect: false
IO.print("abcd".startsWith("a")) // expect: true
IO.print("abcd".startsWith("abcd")) // expect: true
IO.print("abcd".startsWith("abcde")) // expect: false
IO.print("abcd".startsWith("")) // expect: true

// Non-ASCII.
IO.print("søméthîng".startsWith("sømé"))  // expect: true
IO.print("søméthîng".startsWith("some")) // expect: false

// 8-bit clean
IO.print("a\0b\0c".startsWith("a"))  // expect: true
IO.print("a\0b\0c".startsWith("a\0"))  // expect: true
IO.print("a\0b\0c".startsWith("b\0"))  // expect: false
