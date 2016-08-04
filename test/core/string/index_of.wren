System.print("abcd".indexOf("")) // expect: 0
System.print("abcd".indexOf("cd")) // expect: 2
System.print("abcd".indexOf("a")) // expect: 0
System.print("abcd".indexOf("abcd")) // expect: 0
System.print("abcd".indexOf("abcde")) // expect: -1
System.print("abab".indexOf("ab")) // expect: 0

// More complex cases.
System.print("abcdefabcdefg".indexOf("defg")) // expect: 9
System.print("abcdabcdabcd".indexOf("dab")) // expect: 3
System.print("abcdabcdabcdabcd".indexOf("dabcdabc")) // expect: 3
System.print("abcdefg".indexOf("abcdef!")) // expect: -1

// Non-ASCII. Note that it returns byte indices, not code points.
System.print("søméஃthîng".indexOf("e")) // expect: -1
System.print("søméஃthîng".indexOf("m")) // expect: 3
System.print("søméஃthîng".indexOf("thî")) // expect: 9

// 8-bit clean.
System.print("a\0b\0c".indexOf("\0")) // expect: 1
System.print("a\0b\0c".indexOf("a")) // expect: 0
System.print("a\0b\0c".indexOf("b\0c")) // expect: 2
System.print("a\0b\0c".indexOf("a\0b\0c\0d")) // expect: -1
System.print("a\0b\0a\0b".indexOf("a\0b")) // expect: 0
