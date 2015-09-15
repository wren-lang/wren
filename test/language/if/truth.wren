// False and null are false.
if (false) System.print("bad") else System.print("false") // expect: false
if (null) System.print("bad") else System.print("null") // expect: null

// Everything else is true.
if (true) System.print(true) // expect: true
if (0) System.print(0) // expect: 0
if ("") System.print("empty") // expect: empty
