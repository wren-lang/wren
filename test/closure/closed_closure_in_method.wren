var foo = null

{
  var local = "local"
  class Foo {
    method {
      IO.write(local)
    }
  }

  foo = new Foo
}

foo.method // expect: local
