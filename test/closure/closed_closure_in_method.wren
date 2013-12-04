var foo = null

{
  var local = "local"
  class Foo {
    method {
      io.write(local)
    }
  }

  foo = Foo.new
}

foo.method // expect: local
