// TODO: Is this right? Shouldn't it resolve to this.local?
var foo = null

{
  var local = "local"
  class Foo {
    construct new() {}

    method {
      IO.print(local)
    }
  }

  foo = Foo.new()
}

foo.method // expect: local
