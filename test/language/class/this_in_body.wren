class Foo {
  construct new() {}

  System.print(this)

  def toString { "foo" }
}

Foo.new() // expect: foo
