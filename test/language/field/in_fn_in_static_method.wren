class Foo {
  static bar {
    fn () { _field = "wat" } // expect error
  }
}
