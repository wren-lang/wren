class Foo {
  bar {
    var A = "value"
    var A = "other" // expect error
  }
}
