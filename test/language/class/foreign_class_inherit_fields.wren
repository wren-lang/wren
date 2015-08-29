class Foo {
  method() {
    _field = "value"
  }
}

foreign class Bar is Foo {} // expect runtime error: Foreign class 'Bar' may not inherit from a class with fields.
