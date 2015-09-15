{
  var a = "outer"
  {
    System.print(a) // expect: outer
    var a = "inner"
    System.print(a) // expect: inner
  }
}