// False and null are false.
IO.print(false || "ok") // expect: ok
IO.print(null || "ok") // expect: ok

// Everything else is true.
IO.print(true || "ok") // expect: true
IO.print(0 || "ok") // expect: 0
IO.print("s" || "ok") // expect: s
