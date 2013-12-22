class A {
  new(arg) {
    IO.write("new A " + arg)
    _field = arg
  }

  aField { return _field }
}

class B is A {
  new(arg1, arg2) {
    super(arg2)
    IO.write("new B " + arg1)
    _field = arg1
  }

  bField { return _field }
}

class C is B {
  new {
    super("one", "two")
    IO.write("new C")
    _field = "c"
  }

  cField { return _field }
}

var c = new C
// expect: new A two
// expect: new B one
// expect: new C
IO.write(c is A) // expect: true
IO.write(c is B) // expect: true
IO.write(c is C) // expect: true

IO.write(c.aField) // expect: two
IO.write(c.bField) // expect: one
IO.write(c.cField) // expect: c
