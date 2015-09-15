// False and null are false.
System.print(false && "bad") // expect: false
System.print(null && "bad") // expect: null

// Everything else is true.
System.print(true && "ok") // expect: ok
System.print(0 && "ok") // expect: ok
System.print("" && "ok") // expect: ok
