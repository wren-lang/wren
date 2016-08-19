// An empty string is anywhere you look for it.
System.print("abcd".indexOf("", 0)) // expect: 0
System.print("abcd".indexOf("", 1)) // expect: 1
System.print("abcd".indexOf("", 2)) // expect: 2

// Overlapping results.
System.print("aaaaa".indexOf("aaaa", 0)) // expect: 0
System.print("aaaaa".indexOf("aaaa", 1)) // expect: 1
System.print("aaaaa".indexOf("aaaa", 2)) // expect: -1

// It's OK if the needle extends past the end.
System.print("abcd".indexOf("abcde", 0)) // expect: -1
System.print("abcd".indexOf("cde", 3)) // expect: -1

System.print("abcd".indexOf("cd", 0)) // expect: 2
System.print("abcd".indexOf("cd", 1)) // expect: 2
System.print("abcd".indexOf("cd", 2)) // expect: 2
System.print("abcd".indexOf("cd", 3)) // expect: -1

// Negative start.
System.print("abcd".indexOf("cd", -4)) // expect: 2
System.print("abcd".indexOf("cd", -3)) // expect: 2
System.print("abcd".indexOf("cd", -2)) // expect: 2
System.print("abcd".indexOf("cd", -1)) // expect: -1

// Skips past earlier results.
System.print("here as well as here".indexOf("here", 1)) // expect: 16

// Non-ASCII. Note that it returns byte indices, not code points.
System.print("søméஃthîng".indexOf("e", 2)) // expect: -1
System.print("søméஃthîng".indexOf("m", 2)) // expect: 3
System.print("søméஃthîng".indexOf("thî", 8)) // expect: 9

// 8-bit clean.
System.print("a\0b\0c".indexOf("\0", 0)) // expect: 1
System.print("a\0b\0c".indexOf("a", 0)) // expect: 0
System.print("a\0b\0c".indexOf("b\0c", 1)) // expect: 2
System.print("a\0b\0c".indexOf("a\0b\0c\0d", 0)) // expect: -1
System.print("a\0b\0a\0b".indexOf("a\0b", 0)) // expect: 0
