System.print("".trim() == "") // expect: true
System.print("foo".trim() == "foo") // expect: true
System.print(" \t\r\nfoo b\tar \t\r\n".trim() == "foo b\tar") // expect: true
System.print(" \t\r\n \t\r\n".trim() == "") // expect: true
System.print(" \n\n\tsøméஃthîng \n\n\t".trim() == "søméஃthîng") // expect: true

System.print("".trim("abc") == "") // expect: true
System.print("foo".trim("abc") == "foo") // expect: true
System.print("foo".trim("") == "foo") // expect: true
System.print("cbacbfoobarab".trim("abc") == "foobar") // expect: true
System.print("abcbacba".trim("abc") == "") // expect: true
System.print("søméஃthîngsøméஃ".trim("ஃmésø") == "thîng") // expect: true

// 8-bit clean.
System.print(" \t\ra\0b \t\r".trim() == "a\0b") // expect: true
System.print("\0a\0b\0c\0".trim("c\0a") == "b") // expect: true
