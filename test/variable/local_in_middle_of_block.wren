class Foo {
  bar {
    var a = "a"
    IO.write(a) // expect: a
    var b = a + " b"
    IO.write(b) // expect: a b
    var c = a + " c"
    IO.write(c) // expect: a c
    var d = b + " d"
    IO.write(d) // expect: a b d
  }
}

(new Foo).bar