class Foo {}

var foo = new Foo
io.write(foo is Foo) // expect: true

// TODO: Test precedence and grammar of what follows "new".
