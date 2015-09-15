var a = "global"
{
  var a = "shadow"
  System.print(a) // expect: shadow
}
System.print(a) // expect: global