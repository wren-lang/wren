class Foo {
  construct new() { _field = "Foo field" }

  closeOverGet() {
    return Fn.new { _field }
  }

  closeOverSet() {
    return Fn.new { _field = "new value" }
  }
}

var foo = Foo.new()
System.print(foo.closeOverGet()()) // expect: Foo field
foo.closeOverSet()()
System.print(foo.closeOverGet()()) // expect: new value
