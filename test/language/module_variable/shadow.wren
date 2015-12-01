var a = "outer"

{
  var a = "inner"
  System.print(a) // expect: inner
}

System.print(a) // expect: outer
