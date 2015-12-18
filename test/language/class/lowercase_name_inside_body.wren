class foo {
  def construct new() {}

  def static callFoo {
    System.print(foo)
  }

  def callFoo {
    System.print(foo)
  }

  def foo { "instance foo method" }
  def static foo { "static foo method" }
}

foo.callFoo // expect: static foo method
foo.new().callFoo // expect: instance foo method
