// False and null are false.
System.print(false || "arg2") // expect: arg2
System.print(null || "arg2") // expect: arg2

// Everything else is true.
System.print(true || "arg2") // expect: true
System.print(0 || "arg2") // expect: 0
System.print("arg1" || "arg2") // expect: arg1
