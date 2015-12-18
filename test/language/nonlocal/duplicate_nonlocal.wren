class Foo {
  def bar {
    var A = "value"
    var A = "other" // expect error
  }
}
