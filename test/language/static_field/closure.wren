class Foo {
  static initialize() { __field = "Foo field" }

  static closeOverGet() {
    return fn () { __field }
  }

  static closeOverSet() {
    return fn () { __field = "new value" }
  }
}

Foo.initialize()
System.print(Foo.closeOverGet()()) // expect: Foo field
Foo.closeOverSet()()
System.print(Foo.closeOverGet()()) // expect: new value
