class Base {}

class Derived is Base {
  construct new() {}
  foo { super.doesNotExist(1) } // expect runtime error: Base does not implement 'doesNotExist(_)'.
}

Derived.new().foo
