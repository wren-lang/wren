class Foo {}

var foo = Foo.new()
IO.print(foo is Foo) // expect: true

// TODO: Test precedence and grammar of what follows "new".
