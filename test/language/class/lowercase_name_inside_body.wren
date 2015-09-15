class foo {
  construct new() {}

  static callFoo {
    System.print(foo)
  }

  callFoo {
    System.print(foo)
  }

  foo { "instance foo method" }
  static foo { "static foo method" }
}

foo.callFoo // expect: static foo method
foo.new().callFoo // expect: instance foo method
