// Not structurally equal.
IO.print(new Fn { 123 } == new Fn { 123 })  // expect: false
IO.print(new Fn { 123 } != new Fn { 123 })  // expect: true

// Not equal to other types.
IO.print(new Fn { 123 } == 1)         // expect: false
IO.print(new Fn { 123 } == false)     // expect: false
IO.print(new Fn { 123 } == "fn 123")  // expect: false
IO.print(new Fn { 123 } != 1)         // expect: true
IO.print(new Fn { 123 } != false)     // expect: true
IO.print(new Fn { 123 } != "fn 123")  // expect: true

// Equal by identity.
var f = new Fn { 123 }
IO.print(f == f) // expect: true
IO.print(f != f) // expect: false
