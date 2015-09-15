// False and null are false.
System.print(false || "ok") // expect: ok
System.print(null || "ok") // expect: ok

// Everything else is true.
System.print(true || "ok") // expect: true
System.print(0 || "ok") // expect: 0
System.print("s" || "ok") // expect: s
