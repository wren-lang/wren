// Not structurally equal.
IO.print(Fn.new { 123 } == Fn.new { 123 })  // expect: false
IO.print(Fn.new { 123 } != Fn.new { 123 })  // expect: true

// Not equal to other types.
IO.print(Fn.new { 123 } == 1)         // expect: false
IO.print(Fn.new { 123 } == false)     // expect: false
IO.print(Fn.new { 123 } == "fn 123")  // expect: false
IO.print(Fn.new { 123 } != 1)         // expect: true
IO.print(Fn.new { 123 } != false)     // expect: true
IO.print(Fn.new { 123 } != "fn 123")  // expect: true

// Equal by identity.
var f = Fn.new { 123 }
IO.print(f == f) // expect: true
IO.print(f != f) // expect: false
