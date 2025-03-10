class Foo {
  static +(other) { "infix %(this) + %(other)" }
  static -(other) { "infix %(this) - %(other)" }
  static *(other) { "infix %(this) * %(other)" }
  static /(other) { "infix %(this) / %(other)" }
  static %(other) { "infix %(this) \% %(other)" }
  static <(other) { "infix %(this) < %(other)" }
  static >(other) { "infix %(this) > %(other)" }
  static <=(other) { "infix %(this) <= %(other)" }
  static >=(other) { "infix %(this) >= %(other)" }
  static ==(other) { "infix %(this) == %(other)" }
  static !=(other) { "infix %(this) != %(other)" }
  static &(other) { "infix %(this) & %(other)" }
  static |(other) { "infix %(this) | %(other)" }
  static is(other) { "infix %(this) is %(other)" }

  static ! { "prefix !%(this)" }
  static ~ { "prefix ~%(this)" }
  static - { "prefix -%(this)" }
}

System.print(Foo + "a") // expect: infix Foo + a
System.print(Foo - "a") // expect: infix Foo - a
System.print(Foo * "a") // expect: infix Foo * a
System.print(Foo / "a") // expect: infix Foo / a
System.print(Foo % "a") // expect: infix Foo % a
System.print(Foo < "a") // expect: infix Foo < a
System.print(Foo > "a") // expect: infix Foo > a
System.print(Foo <= "a") // expect: infix Foo <= a
System.print(Foo >= "a") // expect: infix Foo >= a
System.print(Foo == "a") // expect: infix Foo == a
System.print(Foo != "a") // expect: infix Foo != a
System.print(Foo & "a") // expect: infix Foo & a
System.print(Foo | "a") // expect: infix Foo | a
System.print(!Foo) // expect: prefix !Foo
System.print(~Foo) // expect: prefix ~Foo
System.print(-Foo) // expect: prefix -Foo
System.print(Foo is "a") // expect: infix Foo is a
