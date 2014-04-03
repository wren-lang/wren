class Foo {
  static bar {
    new Fn { _field = "wat" } // expect error
  }
}
