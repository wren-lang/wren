System.print("something".split("meth")) // expect: [so, ing]
System.print("something".split("some")) // expect: [, thing]
System.print("something".split("ing"))  // expect: [someth, ]
System.print("something".split("math")) // expect: [something]

// Multiple.
System.print("somethingsomething".split("meth")) // expect: [so, ingso, ing]
System.print("abc abc abc".split(" ")) // expect: [abc, abc, abc]
System.print("abcabcabc".split("abc")) // expect: [, , , ]

// Non-ASCII.
System.print("søméthîng".split("méth"))  // expect: [sø, îng]
System.print("søméthîng".split("meth")) // expect: [søméthîng]

// 8-bit clean.
System.print("a\0b\0c".split("\0")) // expect: [a, b, c]
