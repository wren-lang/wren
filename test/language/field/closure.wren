class Foo {
  this new() { _field = "Foo field" }

  closeOverGet {
    return Fn.new { _field }
  }

  closeOverSet {
    return Fn.new { _field = "new value" }
  }
}

var foo = Foo.new()
IO.print(foo.closeOverGet.call()) // expect: Foo field
foo.closeOverSet.call()
IO.print(foo.closeOverGet.call()) // expect: new value
