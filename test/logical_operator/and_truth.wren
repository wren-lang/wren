// False and null are false.
IO.print(false && "bad") // expect: false
IO.print(null && "bad") // expect: null

// Everything else is true.
IO.print(true && "ok") // expect: ok
IO.print(0 && "ok") // expect: ok
IO.print("" && "ok") // expect: ok
