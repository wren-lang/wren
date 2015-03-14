var Nonlocal = "outer"

{
  var Nonlocal = "inner"
  IO.print(Nonlocal) // expect: inner
}

IO.print(Nonlocal) // expect: outer
