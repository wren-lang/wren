class Foo {
  construct new() {}
  var bar
}

var foo = Foo.new()
System.print(foo.bar)           // expect: null
System.print(foo.bar = "value") // expect: value
System.print(foo.bar)           // expect: value

// TODO: Duplicate.
// TODO: static var.
