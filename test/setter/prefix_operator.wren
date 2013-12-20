class Foo {
  bar = value { return value }
}

var foo = new Foo
!foo.bar = "value" // expect error
