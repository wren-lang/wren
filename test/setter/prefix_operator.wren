class Foo {
  bar = value { value }
}

var foo = new Foo
!foo.bar = "value" // expect error
