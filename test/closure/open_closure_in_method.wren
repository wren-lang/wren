{
  var local = "local"
  class Foo {
    method {
      IO.write(local)
    }
  }

  (new Foo).method // expect: local
}
