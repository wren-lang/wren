System.print("".trimEnd() == "") // expect: true
System.print("foo".trimEnd() == "foo") // expect: true
System.print(" \t\r\nfoo b\tar \t\r\n".trimEnd() == " \t\r\nfoo b\tar") // expect: true
System.print(" \t\r\n \t\r\n".trimEnd() == "") // expect: true
System.print("søméஃthîng \n\n\t".trimEnd() == "søméஃthîng") // expect: true

System.print("".trimEnd("abc") == "") // expect: true
System.print("foo".trimEnd("abc") == "foo") // expect: true
System.print("foo".trimEnd("") == "foo") // expect: true
System.print("cbacbfoobarab".trimEnd("abc") == "cbacbfoobar") // expect: true
System.print("abcbacba".trimEnd("abc") == "") // expect: true
System.print("søméஃthîngsøméஃ".trimEnd("ஃmésø") == "søméஃthîng") // expect: true

// 8-bit clean.
System.print(" \t\ra\0b \t\r".trimEnd() == " \t\ra\0b") // expect: true
System.print("\0a\0b\0c\0".trimEnd("c\0") == "\0a\0b") // expect: true
