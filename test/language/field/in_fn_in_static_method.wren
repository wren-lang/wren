class Foo {
  static bar {
    Fn.new { _field = "wat" } // expect error
  }
}
