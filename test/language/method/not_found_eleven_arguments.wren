class Foo {
  construct new() {}
}

Foo.new().someUnknownMethod(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11) // expect runtime error: Foo does not implement 'someUnknownMethod(_,_,_,_,_,_,_,_,_,_,_)'.