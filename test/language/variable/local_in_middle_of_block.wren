class Foo {
  bar {
    var a = "a"
    IO.print(a) // expect: a
    var b = a + " b"
    IO.print(b) // expect: a b
    var c = a + " c"
    IO.print(c) // expect: a c
    var d = b + " d"
    IO.print(d) // expect: a b d
  }
}

Foo.new().bar