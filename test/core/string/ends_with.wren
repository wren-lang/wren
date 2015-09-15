System.print("abcd".endsWith("cd")) // expect: true
System.print("abcd".endsWith("abcde")) // expect: false
System.print("abcd".endsWith("abcd")) // expect: true
System.print("abcd".endsWith("f")) // expect: false
System.print("abcd".endsWith("")) // expect: true

// Non-ASCII.
System.print("søméthîng".endsWith("thîng"))  // expect: true
System.print("søméthîng".endsWith("thing")) // expect: false

// 8-bit clean.
System.print("a\0b\0c".endsWith("\0"))  // expect: false
System.print("a\0b\0c".endsWith("c"))  // expect: true
System.print("a\0b\0c".endsWith("\0c"))  // expect: true
System.print("a\0b\0c".endsWith("\0b"))  // expect: false
