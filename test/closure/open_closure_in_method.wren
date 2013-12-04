{
  var local = "local"
  class Foo {
    method {
      io.write(local)
    }
  }

  Foo.new.method // expect: local
}
