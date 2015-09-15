// Not structurally equal.
System.print(Fn.new { 123 } == Fn.new { 123 })  // expect: false
System.print(Fn.new { 123 } != Fn.new { 123 })  // expect: true

// Not equal to other types.
System.print(Fn.new { 123 } == 1)         // expect: false
System.print(Fn.new { 123 } == false)     // expect: false
System.print(Fn.new { 123 } == "fn 123")  // expect: false
System.print(Fn.new { 123 } != 1)         // expect: true
System.print(Fn.new { 123 } != false)     // expect: true
System.print(Fn.new { 123 } != "fn 123")  // expect: true

// Equal by identity.
var f = Fn.new { 123 }
System.print(f == f) // expect: true
System.print(f != f) // expect: false
