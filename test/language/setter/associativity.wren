class Foo {
  construct new(value) { _value = value }
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
System.print(a.toString) // expect: d
System.print(b.toString) // expect: d
System.print(c.toString) // expect: d
