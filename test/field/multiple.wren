class Foo {
  set(a, b, c, d, e) {
    _a = a
    _b = b
    _c = c
    _d = d
    _e = e
  }
  write {
    io.write(_a)
    io.write(_b)
    io.write(_c)
    io.write(_d)
    io.write(_e)
  }
}

var foo = Foo.new
foo.set(1, 2, 3, 4, 5)
foo.write
// expect: 1
// expect: 2
// expect: 3
// expect: 4
// expect: 5

// TODO(bob): Inherited fields.
// TODO(bob): Trying to get or set a field outside of a class.
// TODO(bob): Fields in nested classes.
// TODO(bob): Closing over fields.
