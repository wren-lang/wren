class foo {
  construct new() {}

  static def callFoo {
    System.print(foo)
  }

  def callFoo {
    System.print(foo)
  }

  def foo { "instance foo method" }
  static def foo { "static foo method" }
}

foo.callFoo // expect: static foo method
foo.new().callFoo // expect: instance foo method
