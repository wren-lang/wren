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

(new C).foo
// expect: C.foo
// expect: A.foo
