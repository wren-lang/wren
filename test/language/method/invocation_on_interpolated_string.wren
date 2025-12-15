class Foo {
  construct new() {}

  static foo { "on static foo" }

  foo        { "on foo" }
}

var signature = "foo"

System.print(Foo.@"%(signature)"()) // expect: on static foo

var foo = Foo.new()

System.print(foo.@"%(signature)"()) // expect: on foo
