class Foo {
  construct new() {}

  +(other) { "infix + " + other }
  -(other) { "infix - " + other }
  *(other) { "infix * " + other }
  /(other) { "infix / " + other }
  %(other) { "infix \% " + other }
  <(other) { "infix < " + other }
  >(other) { "infix > " + other }
  <=(other) { "infix <= " + other }
  >=(other) { "infix >= " + other }
  ==(other) { "infix == " + other }
  !=(other) { "infix != " + other }
  &(other) { "infix & " + other }
  |(other) { "infix | " + other }

  ! { "prefix !" }
  ~ { "prefix ~" }
  - { "prefix -" }
}

var foo = Foo.new()
System.print(foo + "a") // expect: infix + a
System.print(foo - "a") // expect: infix - a
System.print(foo * "a") // expect: infix * a
System.print(foo / "a") // expect: infix / a
System.print(foo % "a") // expect: infix % a
System.print(foo < "a") // expect: infix < a
System.print(foo > "a") // expect: infix > a
System.print(foo <= "a") // expect: infix <= a
System.print(foo >= "a") // expect: infix >= a
System.print(foo == "a") // expect: infix == a
System.print(foo != "a") // expect: infix != a
System.print(foo & "a") // expect: infix & a
System.print(foo | "a") // expect: infix | a
System.print(!foo) // expect: prefix !
System.print(~foo) // expect: prefix ~
System.print(-foo) // expect: prefix -
