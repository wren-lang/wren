// False and null are false.
if (false) IO.print("bad") else IO.print("false") // expect: false
if (null) IO.print("bad") else IO.print("null") // expect: null

// Everything else is true.
if (true) IO.print(true) // expect: true
if (0) IO.print(0) // expect: 0
if ("") IO.print("empty") // expect: empty
