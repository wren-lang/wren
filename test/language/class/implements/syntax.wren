
class Foo implements Object {
  foo { "Bug" }
}

class Bar implements Foo {
  construct new() {}

  foo { "Hello world!" }
}

System.print(Bar.new().foo) // expect: Hello world!

// Check for multiple inheritance
class Countable {
  count               { subclassResponsibility }
}

class Iterable {
  iterate(iter)       { subclassResponsibility }
  iteratorValue(iter) { subclassResponsibility }
}

class EmptyList implements Countable, Iterable {
  construct new()     {}

  count               { 0 }
  iterate(iter)       { false }
  iteratorValue(iter) { Fiber.abort("This should be unreachable") }
}

var emptyList = EmptyList.new()
System.print(emptyList.count) // expect: 0
System.print(emptyList.iterate(null)) // expect: false
