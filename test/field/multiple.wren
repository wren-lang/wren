class Foo {
  set(a, b, c, d, e) {
    _a = a
    _b = b
    _c = c
    _d = d
    _e = e
  }

  write {
    IO.write(_a)
    IO.write(_b)
    IO.write(_c)
    IO.write(_d)
    IO.write(_e)
  }
}

var foo = new Foo
foo.set(1, 2, 3, 4, 5)
foo.write
// expect: 1
// expect: 2
// expect: 3
// expect: 4
// expect: 5
