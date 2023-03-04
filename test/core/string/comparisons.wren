System.print("" < "abc")     // expect: true
System.print("abc" < "def")  // expect: true
System.print("abc" < "abc")  // expect: false
System.print("abc" <= "abc") // expect: true
System.print("ghi" > "")     // expect: true
System.print("ghi" > "abc")  // expect: true
System.print("ghi" > "ghi")  // expect: false
System.print("ghi" >= "ghi") // expect: true

// Non-ascii
System.print("ÀÁA" < "àáa")  // expect: true
System.print("ÀÁA" < "ÀÁA")  // expect: false
System.print("ÀÁA" <= "ÀÁA") // expect: true
System.print("a€E" > "a€e")  // expect: false
System.print("a€E" > "a€E")  // expect: false
System.print("a€E" >= "a€E") // expect: true
