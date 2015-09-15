Fn.new {
  var a = "before"
  System.print(a) // expect: before

  a = "after"
  System.print(a) // expect: after

  System.print(a = "arg") // expect: arg
  System.print(a) // expect: arg
}.call()
