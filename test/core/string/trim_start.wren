System.print("".trimStart() == "") // expect: true
System.print("foo".trimStart() == "foo") // expect: true
System.print(" \t\r\nfoo b\tar \t\r\n".trimStart() == "foo b\tar \t\r\n") // expect: true
System.print(" \t\r\n \t\r\n".trimStart() == "") // expect: true
System.print(" \n\n\tsøméஃthîng".trimStart() == "søméஃthîng") // expect: true

System.print("".trimStart("abc") == "") // expect: true
System.print("foo".trimStart("abc") == "foo") // expect: true
System.print("foo".trimStart("") == "foo") // expect: true
System.print("cbacbfoobarab".trimStart("abc") == "foobarab") // expect: true
System.print("abcbacba".trimStart("abc") == "") // expect: true
System.print("søméஃthîng".trimStart("ஃmésø") == "thîng") // expect: true

// 8-bit clean.
System.print(" \t\ra\0b".trimStart() == "a\0b") // expect: true
System.print("\0a\0b\0c\0".trimStart("a\0") == "b\0c\0") // expect: true
