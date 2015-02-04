IO.print("abcd".endsWith("cd")) // expect: true
IO.print("abcd".endsWith("abcde")) // expect: false
IO.print("abcd".endsWith("abcd")) // expect: true
IO.print("abcd".endsWith("f")) // expect: false
IO.print("abcd".endsWith("")) // expect: true

// Non-ASCII.
IO.print("søméthîng".endsWith("thîng"))  // expect: true
IO.print("søméthîng".endsWith("thing")) // expect: false

// 8-bit clean
IO.print("a\0b\0c".endsWith("\0"))  // expect: false
IO.print("a\0b\0c".endsWith("c"))  // expect: true
IO.print("a\0b\0c".endsWith("\0c"))  // expect: true
IO.print("a\0b\0c".endsWith("\0b"))  // expect: false
