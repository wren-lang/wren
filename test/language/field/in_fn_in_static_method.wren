class Foo {
  static def bar {
    Fn.new { _field = "wat" } // expect error
  }
}
