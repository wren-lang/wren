{
  var a = "local"
  {
    var a = "shadow"
    IO.print(a) // expect: shadow
  }
  IO.print(a) // expect: local
}