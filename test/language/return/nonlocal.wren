class Foo {
  static test() {
    Fn.new {
      return "ok"
    }.call()

    System.print("bad")
  }
}

System.print(Foo.test()) // expect: ok
