// Not structurally equal.
io.write((fn 123) == (fn 123))  // expect: false
io.write((fn 123) != (fn 123))  // expect: true

// Not equal to other types.
io.write((fn 123) == 1)         // expect: false
io.write((fn 123) == false)     // expect: false
io.write((fn 123) == "fn 123")  // expect: false
io.write((fn 123) != 1)         // expect: true
io.write((fn 123) != false)     // expect: true
io.write((fn 123) != "fn 123")  // expect: true

// Equal by identity.
var f = fn 123
io.write(f == f) // expect: true
io.write(f != f) // expect: false
