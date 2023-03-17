
// Check against local upvalues

{
  var foo
  var fn = Fn.new { foo = 42 } // expect runtime error: Object is frozen

  System.print(fn.freeze()) // expect: true
  System.print(fn.call())
}
