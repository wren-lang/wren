var a = "global"
{
  var a = "shadow"
  IO.print(a) // expect: shadow
}
IO.print(a) // expect: global