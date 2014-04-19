class Base {}

class Derived is Base {
  foo { super.doesNotExist } // expect runtime error: Base does not implement method 'doesNotExist'.
}

(new Derived).foo
