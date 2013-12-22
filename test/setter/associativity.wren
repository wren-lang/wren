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
IO.write(a.toString) // expect: d
IO.write(b.toString) // expect: d
IO.write(c.toString) // expect: d
