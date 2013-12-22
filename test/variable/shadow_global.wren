var a = "global"
{
  var a = "shadow"
  IO.write(a) // expect: shadow
}
IO.write(a) // expect: global