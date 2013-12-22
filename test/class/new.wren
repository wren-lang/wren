class Foo {}

var foo = new Foo
IO.write(foo is Foo) // expect: true

// TODO: Test precedence and grammar of what follows "new".
