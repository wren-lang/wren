{
  var local = "local"
  class Foo {
    method {
      io.write(local)
    }
  }

  (new Foo).method // expect: local
}
