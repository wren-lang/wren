class A {
  foo {
    IO.print("A.foo")
  }
}

class B is A {}

class C is B {
  foo {
    IO.print("C.foo")
    super.foo
  }
}

C.new().foo
// expect: C.foo
// expect: A.foo
