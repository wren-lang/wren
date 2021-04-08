class Baz {
  construct new() {}
}

class Bar {
  construct new() {
  }
}

class Foo {
  construct new() {
    return
  }
}
System.print(Baz.new()) // expect: instance of Baz
System.print(Bar.new()) // expect: instance of Bar
System.print(Foo.new()) // expect: instance of Foo
System.print(Foo.new() != null) // expect: true
