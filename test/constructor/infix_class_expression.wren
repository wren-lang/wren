class Foo {
  + other { return "Foo " + other }
}

io.write(new Foo + "value") // expect: Foo value

// TODO: Other expressions following a constructor, like new Foo.bar("arg").
