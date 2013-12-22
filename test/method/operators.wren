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
IO.write(foo + "a") // expect: infix + a
IO.write(foo - "a") // expect: infix - a
IO.write(foo * "a") // expect: infix * a
IO.write(foo / "a") // expect: infix / a
IO.write(foo % "a") // expect: infix % a
IO.write(foo < "a") // expect: infix < a
IO.write(foo > "a") // expect: infix > a
IO.write(foo <= "a") // expect: infix <= a
IO.write(foo >= "a") // expect: infix >= a
IO.write(foo == "a") // expect: infix == a
IO.write(foo != "a") // expect: infix != a
IO.write(!foo) // expect: prefix !
IO.write(-foo) // expect: prefix -
