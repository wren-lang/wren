var foo = null

{
  var local = "local"
  class Foo {
    method {
      IO.print(local)
    }
  }

  foo = new Foo
}

foo.method // expect: local
