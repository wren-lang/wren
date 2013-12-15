{
  var a = "outer"
  {
    io.write(a) // expect: outer
    var a = "inner"
    io.write(a) // expect: inner
  }
}