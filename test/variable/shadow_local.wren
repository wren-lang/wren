{
  var a = "local"
  {
    var a = "shadow"
    io.write(a) // expect: shadow
  }
  io.write(a) // expect: local
}