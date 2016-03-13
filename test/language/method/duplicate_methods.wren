class Foo {
  construct new() {}
  foo() {}
  foo() {} // expect error
  
  static bar() {}
  static bar() {} // expect error
  
  baz() {}
  static baz() {}
}
