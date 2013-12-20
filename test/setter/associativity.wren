class Foo {
  new(value) { _value = value }
  toString { return _value }
  bar = value {
    _value = value
    return value
  }
}

var a = new Foo("a")
var b = new Foo("b")
var c = new Foo("c")

// Assignment is right-associative.
a.bar = b.bar = c.bar = "d"
io.write(a.toString) // expect: d
io.write(b.toString) // expect: d
io.write(c.toString) // expect: d
