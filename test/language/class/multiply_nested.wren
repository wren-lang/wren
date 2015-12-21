class A {
  construct new() {}

  class B {
    construct new() {}

    class C {
      construct new() {}

      class D {
        construct new() {}

        def test() { System.print("ok") }
      }
    }
  }
}

A.new().B.new().C.new().D.new().test() // expect: ok
