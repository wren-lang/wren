{
  var a = "outer"
  {
    var a = a + " inner"
    IO.print(a) // expect: outer inner
  }
}
