var a = "global"
{
  var a = "shadow"
  io.write(a) // expect: shadow
}
io.write(a) // expect: global