
class Bar {
  call() { "super call()" }
}

class Foo is Bar {
  construct new() {}

  call() { super.@"call()"() }
}

var foo = Foo.new()

System.print(foo.call()) // expect: super call()
