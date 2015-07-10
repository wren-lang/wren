class Foo {
  static this new() {} // expect error
  this static new() {} // expect error
}
