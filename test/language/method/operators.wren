class Foo {
  def construct new() {}

  def +(other) { "infix + %(other)" }
  def -(other) { "infix - %(other)" }
  def *(other) { "infix * %(other)" }
  def /(other) { "infix / %(other)" }
  def %(other) { "infix \% %(other)" }
  def <(other) { "infix < %(other)" }
  def >(other) { "infix > %(other)" }
  def <=(other) { "infix <= %(other)" }
  def >=(other) { "infix >= %(other)" }
  def ==(other) { "infix == %(other)" }
  def !=(other) { "infix != %(other)" }
  def &(other) { "infix & %(other)" }
  def |(other) { "infix | %(other)" }
  def is(other) { "infix is %(other)" }

  def ! { "prefix !" }
  def ~ { "prefix ~" }
  def - { "prefix -" }
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
System.print(foo is "a") // expect: infix is a
