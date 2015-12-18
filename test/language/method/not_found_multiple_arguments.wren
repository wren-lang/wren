class Foo {
  def construct new() {}
}

Foo.new().someUnknownMethod(1, 2) // expect runtime error: Foo does not implement 'someUnknownMethod(_,_)'.