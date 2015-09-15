class Foo {
  construct new() { _field = "Foo field" }

  closeOverGet {
    return Fn.new { _field }
  }

  closeOverSet {
    return Fn.new { _field = "new value" }
  }
}

var foo = Foo.new()
System.print(foo.closeOverGet.call()) // expect: Foo field
foo.closeOverSet.call()
System.print(foo.closeOverGet.call()) // expect: new value
