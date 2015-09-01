class Foo {
  construct new() {}
}

Foo.new().someUnknownMethod // expect runtime error: Foo does not implement 'someUnknownMethod'.
