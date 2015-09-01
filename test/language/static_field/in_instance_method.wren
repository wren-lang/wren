class Foo {
  construct new() {}

  set(a, b, c, d, e) {
    __a = a
    __b = b
    __c = c
    __d = d
    __e = e
  }

  write() {
    IO.print(__a)
    IO.print(__b)
    IO.print(__c)
    IO.print(__d)
    IO.print(__e)
  }
}

Foo.new().set(1, 2, 3, 4, 5)
Foo.new().write()
// expect: 1
// expect: 2
// expect: 3
// expect: 4
// expect: 5
