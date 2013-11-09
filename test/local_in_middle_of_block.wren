class Foo {
  bar {
    var a = "a"
    io.write(a) // expect: a
    var b = a + " b"
    io.write(b) // expect: a b
    var c = a + " c"
    io.write(c) // expect: a c
    var d = b + " d"
    io.write(d) // expect: a b d
  }
}

Foo.new.bar