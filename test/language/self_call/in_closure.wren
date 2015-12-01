class Foo {
  construct new(field) {
    _field = field
  }

  method() {
    System.print(_field)
  }

  makeClosure() { Fn.new { @method() } }
}

Foo.new("value").makeClosure().call() // expect: value
