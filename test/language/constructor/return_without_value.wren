class Foo {
  construct new() {
    return
  }
}
System.print(Foo.new()) // expect: instance of Foo
System.print(Foo.new() != null) // expect: true
