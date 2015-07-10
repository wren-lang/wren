class Foo {
  this new(value) { _value = value }
  toString { _value }
  bar=(value) {
    _value = value
    return value
  }
}

var a = Foo.new("a")
var b = Foo.new("b")
var c = Foo.new("c")

// Assignment is right-associative.
a.bar = b.bar = c.bar = "d"
IO.print(a.toString) // expect: d
IO.print(b.toString) // expect: d
IO.print(c.toString) // expect: d
