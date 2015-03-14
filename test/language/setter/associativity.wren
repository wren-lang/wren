class Foo {
  new(value) { _value = value }
  toString { _value }
  bar=(value) {
    _value = value
    return value
  }
}

var a = new Foo("a")
var b = new Foo("b")
var c = new Foo("c")

// Assignment is right-associative.
a.bar = b.bar = c.bar = "d"
IO.print(a.toString) // expect: d
IO.print(b.toString) // expect: d
IO.print(c.toString) // expect: d
