
class Foo implements Object {
  foo { "Bug" }
}

class Bar implements Foo {
} // expect runtime error: Bar does not implement 'foo'.
