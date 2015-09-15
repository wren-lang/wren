class Foo {
  construct new() {}

  bar {
    var a = "a"
    System.print(a) // expect: a
    var b = a + " b"
    System.print(b) // expect: a b
    var c = a + " c"
    System.print(c) // expect: a c
    var d = b + " d"
    System.print(d) // expect: a b d
  }
}

Foo.new().bar