class A {
  foo {
    io.write("A.foo")
  }
}

class B is A {}

class C is B {
  foo {
    io.write("C.foo")
    super.foo
  }
}

(new C).foo
// expect: C.foo
// expect: A.foo
