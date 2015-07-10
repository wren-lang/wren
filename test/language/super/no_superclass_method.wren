class Base {}

class Derived is Base {
  foo { super.doesNotExist(1) } // expect runtime error: Base does not implement 'doesNotExist(_)'.
}

Derived.new().foo
