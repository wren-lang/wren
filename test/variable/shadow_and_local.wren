{
  var a = "outer"
  {
    IO.print(a) // expect: outer
    var a = "inner"
    IO.print(a) // expect: inner
  }
}