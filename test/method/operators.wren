class Foo {
  + other { return "infix + " + other }
  - other { return "infix - " + other }
  * other { return "infix * " + other }
  / other { return "infix / " + other }
  % other { return "infix % " + other }
  < other { return "infix < " + other }
  > other { return "infix > " + other }
  <= other { return "infix <= " + other }
  >= other { return "infix >= " + other }
  == other { return "infix == " + other }
  != other { return "infix != " + other }

  ! { return "prefix !" }
  - { return "prefix -" }
}

var foo = new Foo
IO.print(foo + "a") // expect: infix + a
IO.print(foo - "a") // expect: infix - a
IO.print(foo * "a") // expect: infix * a
IO.print(foo / "a") // expect: infix / a
IO.print(foo % "a") // expect: infix % a
IO.print(foo < "a") // expect: infix < a
IO.print(foo > "a") // expect: infix > a
IO.print(foo <= "a") // expect: infix <= a
IO.print(foo >= "a") // expect: infix >= a
IO.print(foo == "a") // expect: infix == a
IO.print(foo != "a") // expect: infix != a
IO.print(!foo) // expect: prefix !
IO.print(-foo) // expect: prefix -
