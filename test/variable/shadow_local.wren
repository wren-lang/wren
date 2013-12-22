{
  var a = "local"
  {
    var a = "shadow"
    IO.write(a) // expect: shadow
  }
  IO.write(a) // expect: local
}