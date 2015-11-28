class Foo {
  construct new() { _field = "Foo field" }

  closeOverGet() {
    return fn () { _field }
  }

  closeOverSet() {
    return fn () { _field = "new value" }
  }
}

var foo = Foo.new()
System.print(foo.closeOverGet()()) // expect: Foo field
foo.closeOverSet()()
System.print(foo.closeOverGet()()) // expect: new value
