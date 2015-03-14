{
  var local = "local"
  class Foo {
    method {
      IO.print(local)
    }
  }

  (new Foo).method // expect: local
}
