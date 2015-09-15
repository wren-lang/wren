{
  var a = "outer"
  {
    var a = a + " inner"
    System.print(a) // expect: outer inner
  }
}
