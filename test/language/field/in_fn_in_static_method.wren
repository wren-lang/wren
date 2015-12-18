class Foo {
  def static bar {
    Fn.new { _field = "wat" } // expect error
  }
}
