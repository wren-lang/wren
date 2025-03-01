
// Check against global upvalues

var foo
var fn = Fn.new { foo = 42 }

System.print(fn.setFrozen(true)) // expect: true
System.print(fn.call())          // expect: 42
