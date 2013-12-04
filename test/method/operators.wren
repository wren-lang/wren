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

var foo = Foo.new
io.write(foo + "a") // expect: infix + a
io.write(foo - "a") // expect: infix - a
io.write(foo * "a") // expect: infix * a
io.write(foo / "a") // expect: infix / a
io.write(foo % "a") // expect: infix % a
io.write(foo < "a") // expect: infix < a
io.write(foo > "a") // expect: infix > a
io.write(foo <= "a") // expect: infix <= a
io.write(foo >= "a") // expect: infix >= a
io.write(foo == "a") // expect: infix == a
io.write(foo != "a") // expect: infix != a
io.write(!foo) // expect: prefix !
io.write(-foo) // expect: prefix -
