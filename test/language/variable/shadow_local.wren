{
  var a = "local"
  {
    var a = "shadow"
    System.print(a) // expect: shadow
  }
  System.print(a) // expect: local
}