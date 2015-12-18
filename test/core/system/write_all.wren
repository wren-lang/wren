class Foo {
  construct new() {}

  def toString { "Foo" }
}

System.writeAll([]) // expect:
System.print()
System.writeAll([1, true, Foo.new(), "s"]) // expect: 1trueFoos
System.print()
