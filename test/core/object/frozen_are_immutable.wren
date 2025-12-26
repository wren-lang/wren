
class Foo {
  construct new() { freeze() }
  mutate() { _value = 42 } // expect runtime error: Object is frozen
}

System.print(Foo.new().mutate())
