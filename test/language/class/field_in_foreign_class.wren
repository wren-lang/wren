foreign class Foo {
  bar {
    // Can't read a field.
    IO.print(_bar) // expect error

    // Or write one.
    _bar = "value" // expect error
  }
}
