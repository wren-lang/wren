class Foo {
  construct new() {}
  var bar = "init"
}

var foo = Foo.new()
System.print(foo.bar)           // expect: init
System.print(foo.bar = "value") // expect: value
System.print(foo.bar)           // expect: value
