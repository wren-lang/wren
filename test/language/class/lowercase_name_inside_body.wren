class foo {
  static callFoo {
    IO.print(foo)
  }

  callFoo {
    IO.print(foo)
  }

  foo { "instance foo method" }
  static foo { "static foo method" }
}

foo.callFoo // expect: static foo method
(new foo).callFoo // expect: instance foo method
