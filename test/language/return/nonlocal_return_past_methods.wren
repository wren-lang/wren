class Foo {
  static test() {
    a {
      return "ok"
    }
    System.print("bad")
  }

  static a(fn) {
    System.print("a")
    b(fn)
    System.print("bad")
  }

  static b(fn) {
    System.print("b")
    c(fn)
    System.print("bad")
  }

  static c(fn) {
    System.print("c")
    fn.call()
    System.print("bad")
  }
}

System.print(Foo.test())
// expect: a
// expect: b
// expect: c
// expect: ok
