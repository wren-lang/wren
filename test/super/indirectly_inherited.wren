class A {
  foo {
    IO.write("A.foo")
  }
}

class B is A {}

class C is B {
  foo {
    IO.write("C.foo")
    super.foo
  }
}

(new C).foo
// expect: C.foo
// expect: A.foo
