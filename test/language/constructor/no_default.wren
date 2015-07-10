class Foo {
  this real() {}
}

// Classes do not get an argument-less "new()" if they define a constructor.
var foo = Foo.new() // expect runtime error: Foo metaclass does not implement 'new()'.
