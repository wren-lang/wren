
var fn = Fn.new { 42 }

System.print(fn.freeze()) // expect: true
System.print(fn.isFrozen) // expect: true
System.print(fn.call())   // expect: 42
