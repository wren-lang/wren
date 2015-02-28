class Foo {
  new { _field = "Foo field" }

  closeOverGet {
    return new Fn { _field }
  }

  closeOverSet {
    return new Fn { _field = "new value" }
  }
}

var foo = new Foo
IO.print(foo.closeOverGet.call()) // expect: Foo field
foo.closeOverSet.call()
IO.print(foo.closeOverGet.call()) // expect: new value
