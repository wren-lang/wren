class A {
  construct new(arg) {
    IO.print("new A ", arg)
    _field = arg
  }

  aField { _field }
}

class B is A {
  construct new(arg1, arg2) {
    super(arg2)
    IO.print("new B ", arg1)
    _field = arg1
  }

  bField { _field }
}

class C is B {
  construct new() {
    super("one", "two")
    IO.print("new C")
    _field = "c"
  }

  cField { _field }
}

var c = C.new()
// expect: new A two
// expect: new B one
// expect: new C
IO.print(c is A) // expect: true
IO.print(c is B) // expect: true
IO.print(c is C) // expect: true

IO.print(c.aField) // expect: two
IO.print(c.bField) // expect: one
IO.print(c.cField) // expect: c
