
class Foo {
  construct new() {}

  call()          { "call()" }
  indirect_call() { this.@"call()"() }
}

var foo = Foo.new()

System.print(foo.indirect_call()) // expect: call()
