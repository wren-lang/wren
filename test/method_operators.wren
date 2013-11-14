class Foo {
  + other { "infix + " + other }
  - other { "infix - " + other }
  * other { "infix * " + other }
  / other { "infix / " + other }
  % other { "infix % " + other }
  < other { "infix < " + other }
  > other { "infix > " + other }
  <= other { "infix <= " + other }
  >= other { "infix >= " + other }
  == other { "infix == " + other }
  != other { "infix != " + other }

  ! { "prefix !" }
  - { "prefix -" }
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
