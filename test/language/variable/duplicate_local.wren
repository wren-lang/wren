class Foo {
  def bar {
    var a = "value"
    var a = "other" // expect error
  }
}
