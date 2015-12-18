class Foo {
  construct new() {}

  def toString { "Foo" }
}

System.printAll([]) // expect:
System.printAll([1, true, Foo.new(), "s"]) // expect: 1trueFoos
