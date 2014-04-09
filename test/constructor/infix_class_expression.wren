class Foo {
  +(other) { "Foo " + other }
}

IO.print(new Foo + "value") // expect: Foo value

// TODO: Other expressions following a constructor, like new Foo.bar("arg").
