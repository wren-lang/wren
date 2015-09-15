var Nonlocal = "outer"

{
  var Nonlocal = "inner"
  System.print(Nonlocal) // expect: inner
}

System.print(Nonlocal) // expect: outer
