var foo = null

{
  var local = "local"
  class Foo {
    method {
      io.write(local)
    }
  }

  foo = new Foo
}

foo.method // expect: local
