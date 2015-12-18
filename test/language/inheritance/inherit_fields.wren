class Foo {
  construct new() {}

  def foo(a, b) {
    _field1 = a
    _field2 = b
  }

  def fooPrint {
    System.print(_field1)
    System.print(_field2)
  }
}

class Bar is Foo {
  construct new() {}

  def bar(a, b) {
    _field1 = a
    _field2 = b
  }

  def barPrint {
    System.print(_field1)
    System.print(_field2)
  }
}

var bar = Bar.new()
bar.foo("foo 1", "foo 2")
bar.bar("bar 1", "bar 2")

bar.fooPrint
// expect: foo 1
// expect: foo 2

bar.barPrint
// expect: bar 1
// expect: bar 2
