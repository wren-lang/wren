
var fn = Fn.new { 42 }

System.print(fn.isFrozen) // expect: false
System.print(fn.call())   // expect: 42
