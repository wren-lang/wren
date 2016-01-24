class Foo {
  static test() {
    System.print("1")
    Fn.new {
      System.print("2")
      Fn.new {
        System.print("3")
        Fn.new {
          return "ok"
        }.call()
        System.print("bad")
      }.call()
      System.print("bad")
    }.call()
    System.print("bad")
  }
}

System.print(Foo.test())
// expect: 1
// expect: 2
// expect: 3
// expect: ok
