{
  var a = "outer"
  {
    IO.write(a) // expect: outer
    var a = "inner"
    IO.write(a) // expect: inner
  }
}