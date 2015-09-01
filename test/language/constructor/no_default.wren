class Foo {}

// Classes do not get a constructor by default.
var foo = Foo.new() // expect runtime error: Foo metaclass does not implement 'new()'.
