class Foo {
  construct new() {}
  foo() {}
  foo() {} // expect error

  static bar() {}
  static bar() {} // expect error

  // No error on instance and static method with same signature.
  baz() {}
  static baz() {}
}
