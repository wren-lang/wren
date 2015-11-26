class Foo {
  static initialize() { __field = "Foo field" }

  static closeOverGet() {
    return Fn.new { __field }
  }

  static closeOverSet() {
    return Fn.new { __field = "new value" }
  }
}

Foo.initialize()
System.print(Foo.closeOverGet()()) // expect: Foo field
Foo.closeOverSet()()
System.print(Foo.closeOverGet()()) // expect: new value
