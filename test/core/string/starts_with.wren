System.print("abcd".startsWith("cd")) // expect: false
System.print("abcd".startsWith("a")) // expect: true
System.print("abcd".startsWith("abcd")) // expect: true
System.print("abcd".startsWith("abcde")) // expect: false
System.print("abcd".startsWith("")) // expect: true

// Non-ASCII.
System.print("søméthîng".startsWith("sømé"))  // expect: true
System.print("søméthîng".startsWith("some")) // expect: false

// 8-bit clean.
System.print("a\0b\0c".startsWith("a"))  // expect: true
System.print("a\0b\0c".startsWith("a\0"))  // expect: true
System.print("a\0b\0c".startsWith("b\0"))  // expect: false
