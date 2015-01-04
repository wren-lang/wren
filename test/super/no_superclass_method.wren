class Base {}

class Derived is Base {
  foo { super.doesNotExist } // expect runtime error: Base does not implement method 'doesNotExist' with 0 arguments.
}

(new Derived).foo
