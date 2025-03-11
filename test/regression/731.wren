
Fn.new {
  var foo
  System.print(foo) // expect: null
}.call("Bug")
