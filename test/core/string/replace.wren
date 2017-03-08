System.print("something".replace("some", "no")) // expect: nothing
System.print("something".replace("thing", "one")) // expect: someone
System.print("something".replace("ometh", "umm"))  // expect: summing
System.print("something".replace("math", "ton")) // expect: something

// Multiple.
System.print("somethingsomething".replace("some", "no")) // expect: nothingnothing
System.print("abc abc abc".replace(" ", "")) // expect: abcabcabc
System.print("abcabcabc".replace("abc", "")) // expect: 

// Non-ASCII.
System.print("søméthîng".replace("sømé", "nø"))  // expect: nøthîng
System.print("søméthîng".replace("meth", "ton")) // expect: søméthîng

// 8-bit clean.
System.print("a\0b\0c".replace("\0", "")) // expect: abc
System.print("a\0b\0c".replace("b", "") == "a\0\0c") // expect: true
