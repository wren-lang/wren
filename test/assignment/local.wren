new Fn {
  var a = "before"
  IO.print(a) // expect: before

  a = "after"
  IO.print(a) // expect: after

  IO.print(a = "arg") // expect: arg
  IO.print(a) // expect: arg
}.call
