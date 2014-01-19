class Foo {
  static initialize { __field = "Foo field" }

  static closeOverGet {
    return fn { return __field }
  }

  static closeOverSet {
    return fn { __field = "new value" }
  }
}

Foo.initialize
IO.print(Foo.closeOverGet.call) // expect: Foo field
Foo.closeOverSet.call
IO.print(Foo.closeOverGet.call) // expect: new value
