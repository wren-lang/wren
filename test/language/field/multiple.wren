class Foo {
  set(a, b, c, d, e) {
    _a = a
    _b = b
    _c = c
    _d = d
    _e = e
  }

  write {
    IO.print(_a)
    IO.print(_b)
    IO.print(_c)
    IO.print(_d)
    IO.print(_e)
  }
}

var foo = Foo.new()
foo.set(1, 2, 3, 4, 5)
foo.write
// expect: 1
// expect: 2
// expect: 3
// expect: 4
// expect: 5
